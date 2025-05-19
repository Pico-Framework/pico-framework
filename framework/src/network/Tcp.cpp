#include "network/Tcp.h"

#include <lwip/dns.h>
#include <lwip/api.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <cstring>
#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <pico/stdlib.h>
#include "utility/utility.h"
#include "network/lwip_dns_resolver.h"

#if PICO_TCP_ENABLE_TLS
#include <lwip/altcp.h>
#include <lwip/altcp_tls.h>
#endif

#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(Tcp);

Tcp::Tcp()
    : sockfd(-1), connected(false), use_tls(false)
#if PICO_TCP_ENABLE_TLS
      ,
      tls_config(nullptr), server_tls_config(nullptr),
      tls_pcb(nullptr)
#endif
{
}

Tcp::Tcp(int fd)
    : sockfd(fd), connected(true), use_tls(false)
#if PICO_TCP_ENABLE_TLS
      ,
      tls_config(nullptr), server_tls_config(nullptr),
      tls_pcb(nullptr)
#endif
{
}

Tcp::~Tcp()
{
    close();
}

Tcp::Tcp(Tcp &&other) noexcept
{
    *this = std::move(other);
}

Tcp &Tcp::operator=(Tcp &&other) noexcept
{
    if (this != &other)
    {
        sockfd = other.sockfd;
    #if PICO_TCP_ENABLE_TLS
        tls_pcb = other.tls_pcb;
        tls_config = other.tls_config;
        server_tls_config = other.server_tls_config;
    #endif
        connected = other.connected;
        use_tls = other.use_tls;
        recv_buffer = other.recv_buffer;

        other.sockfd = -1;
    #if PICO_TCP_ENABLE_TLS
        other.tls_pcb = nullptr;
        other.tls_config = nullptr;
        other.server_tls_config = nullptr;
    #endif    
        other.recv_buffer = nullptr;
    }
    return *this;
}

std::string Tcp::getPeerIp() const
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (lwip_getpeername(sockfd, reinterpret_cast<sockaddr *>(&addr), &len) == 0)
    {
        ip_addr_t ip;
        ip.addr = addr.sin_addr.s_addr;
        return std::string(ipaddr_ntoa(&ip));
    }
    return "0.0.0.0";
}

#if PICO_TCP_ENABLE_TLS
void Tcp::setRootCACertificate(const std::string &pem)
{
    root_ca_cert = pem;
}


void Tcp::setServerTlsConfig(const std::string &cert, const std::string &key)
{
    server_tls_cert = cert;
    server_tls_key = key;

    server_tls_config = altcp_tls_create_config_server_privkey_cert(
        reinterpret_cast<const uint8_t *>(server_tls_key.c_str()), server_tls_key.size(),
        reinterpret_cast<const uint8_t *>(server_tls_cert.c_str()), server_tls_cert.size(),
        nullptr, 0 // no passphrase
    );
    if (!server_tls_config)
    {
        printf("[Tcp] Failed to create TLS server config\n");
    }
}
#endif

bool Tcp::connect(const char *host, int port, bool use_tls)
{
    strncpy(hostname, host, sizeof(hostname) - 1);
    this->use_tls = use_tls;
    ip_addr_t ip;
    if (!resolveHostnameBlocking(host, &ip))
    {
        printf("[Tcp] DNS resolution failed for %s\n", host);
        return false;
    }
#if PICO_TCP_ENABLE_TLS
    return use_tls ? connectTls(ip, port) : connectPlain(ip, port);
#else
    return connectPlain(ip, port);
#endif
}

bool Tcp::connectPlain(const ip_addr_t &ip, int port)
{
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = ip.addr;

    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[Tcp] Failed to create socket\n");
        return false;
    }

    if (lwip_connect(sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        printf("[Tcp] Failed to connect to server\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }
    TRACE("[Tcp] Connected to server (plain)\n");
    connected = true;
    use_tls = false;
    return true;
}

#if PICO_TCP_ENABLE_TLS
bool Tcp::connectTls(const char *host, int port)
{
    ip_addr_t ip;
    if (!resolveHostnameBlocking(host, &ip))
    {
        printf("[Tcp] DNS resolution failed for %s\n", host);
        return false;
    }
    return connectTls(ip, port);
}

err_t Tcp::onConnected(void *arg, struct altcp_pcb *conn, err_t err)
{
    TRACE("[Tcp] onConnected callback\n");
    if (err != ERR_OK)
    {
        printf("[Tcp] Connection failed: %d\n", err);

        return err;
    }
    // Set the context for the callbacks
    auto *self = static_cast<Tcp *>(arg);

    self->connectResult = err;

    if (self->connectingTask)
    {
        TRACE("TLS Connection State in onConnect: %d\n", conn->state);
        xTaskNotifyGiveIndexed(self->connectingTask, NotifyConnect);
    }

    return ERR_OK;
}

void Tcp::onError(void *arg, err_t err)
{
    auto *self = static_cast<Tcp *>(arg);
    printf("[Tcp] altcp error: %d\n", err);

    self->connectResult = err;

    // Notify the waiting task (e.g. to break ulTaskNotifyTakeIndexed)
    if (self->connectingTask)
    {
        xTaskNotifyGiveIndexed(self->connectingTask, NotifyConnect);
    }

    self->tls_pcb = nullptr;
}

bool Tcp::connectTls(const ip_addr_t &ip, int port)
{
    if (!tls_config)
    {
        TRACE("[Tcp] RootCA Certificate is: %s\n", root_ca_cert.c_str());
        tls_config = altcp_tls_create_config_client(
            reinterpret_cast<const uint8_t *>(root_ca_cert.c_str()),
            root_ca_cert.size() + 1); // +1 for null terminator
        // tls_config = altcp_tls_create_config_client(nullptr, 0);
        if (!tls_config)
        {
            printf("[Tcp] TLS config creation failed %d\n", tls_config);
            return false;
        }
        TRACE("[Tcp] TLS config created\n");
    }
    tls_pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    // tls_pcb = altcp_tls_new(tls_config, IP_GET_TYPE(&ip));
    if (!tls_pcb)
    {
        printf("[Tcp] Failed to create TLS connection\n");
        return false;
    }
    TRACE("Setting tls hostname: %s\n", hostname);

    // Because altcp_tls_context() returns void*, we must cast to correct type in C++
    auto *ssl_ctx = static_cast<mbedtls_ssl_context *>(altcp_tls_context(tls_pcb));
    mbedtls_ssl_set_hostname(ssl_ctx, hostname);

    // Register recv callback *before* connect
    altcp_arg(tls_pcb, this); // Set the context for the callbacks
    TRACE("[Tcp] Registering TLS recv callback\n");
    altcp_recv(tls_pcb, tlsRecvCallback); // Register callback

    this->connectResult = ERR_OK;
    this->connectingTask = xTaskGetCurrentTaskHandle();
    altcp_err(tls_pcb, &Tcp::onError);
    TRACE("[Tcp] TLS Connecting to %s:%d\n", ipaddr_ntoa(&ip), port);
    err_t err = altcp_connect(tls_pcb, &ip, port, &Tcp::onConnected);
    if (err != ERR_OK)
    {
        printf("[Tcp] altcp_connect failed: %d\n", err);
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
        return false;
    }

    // Wait briefly for the handshake — altcp doesn't expose state.
    // Wait for notification that TLS handshake completed
    ulTaskNotifyTakeIndexed(NotifyConnect, pdTRUE, pdMS_TO_TICKS(portMAX_DELAY));
    TRACE("[Tcp] TLS handshake completed\n");
    if (connectResult == ERR_OK)
    {
        printf("[Tcp] TLS connection established\n");
        connected = true;
    }
    else
    {
        printf("[Tcp] TLS connection failed %d\n", connectResult);
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
        return false;
    }
    use_tls = true;
    return true;
}
#endif

int Tcp::send(const char *buffer, size_t size)
{
    TRACE("[Tcp] Sending %zu bytes\n", size);
#if PICO_TCP_ENABLE_TLS
    if (!buffer || size == 0 || !connected || (use_tls && !tls_pcb))
#else
    if (!buffer || size == 0 || !connected)
#endif
    {
        printf("[Tcp] Invalid buffer, size, or connection\n");
        return -1;
    }

    constexpr size_t chunkSize = HTTP_BUFFER_SIZE;
    size_t totalSent = 0;

    // absolute_time_t startTime = get_absolute_time(); // <-- your timing starts here

    while (totalSent < size)
    {
        size_t toSend = (size - totalSent > chunkSize) ? chunkSize : (size - totalSent);
    #if PICO_TCP_ENABLE_TLS
        if (use_tls && tls_pcb)
        {

            if (tls_pcb->state == nullptr)
            {
                printf("[Tcp] TLS connection is not established\n");
                return -1;
            }

            err_t err = altcp_write(tls_pcb, buffer + totalSent, toSend, TCP_WRITE_FLAG_COPY);
            if (err != ERR_OK)
            {
                printf("[Tcp] altcp_write failed: %d\n", err);
                return -1;
            }
            if (altcp_output(tls_pcb) != ERR_OK)
            {
                printf("[Tcp] altcp_output failed\n");
                return -1;
            }

        else
    #endif        
        if (sockfd >= 0)
        {
            int ret = lwip_send(sockfd, buffer + totalSent, toSend, 0);
            if (ret <= 0)
            {
                warning("[Tcp] lwip_send failed: ", ret);
                return -1;
            }
        }
        else
        {
            printf("[Tcp] No valid socket or TLS connection\n");
            return -1;
        }

        vTaskDelay(pdMS_TO_TICKS(STREAM_SEND_DELAY_MS)); // Throttle per chunk
        totalSent += toSend;
    }

    // After sending ALL chunks: yield and delay for TCP flush
    taskYIELD();
    vTaskDelay(pdMS_TO_TICKS(20)); // Give lwIP time to transmit the data

    return static_cast<int>(size); // Report success
}

err_t Tcp::tlsRecvCallback(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err)
{
    auto *self = static_cast<Tcp *>(arg);
    if (!self)
        return ERR_VAL;

    if (!p)
    {
        // Remote closed connection
        if (self->recv_buffer)
        {
            pbuf_free(self->recv_buffer);
            self->recv_buffer = nullptr;
        }
        self->recv_offset = 0;
        return ERR_OK;
    }

    // Drop if already have a pbuf (single-buffer policy for now)
    if (self->recv_buffer)
    {
        pbuf_free(p); // Drop additional pbuf to prevent overflow
        return ERR_OK;
    }

    self->recv_buffer = p;
    self->recv_offset = 0;

    // Notify any task waiting in recv()
    if (self->waiting_task)
    {
        xTaskNotifyGiveIndexed(self->waiting_task, NotifyRecv);
        self->waiting_task = nullptr;
    }

    return ERR_OK;
}

int Tcp::recv(char *buffer, size_t size, uint32_t timeout_ms)
{
    if (!use_tls)
    {
        TRACE("Plain recv: %zu bytes\n", size);
        int received = lwip_recv(sockfd, buffer, size, 0);
        if (received < 0)
        {
            TRACE("Plain recv error: %d, %s\n", errno, strerror(errno));
            return received;
        }
        return received;
    }

#if PICO_TCP_ENABLE_TLS
    if (!tls_pcb || !buffer || size == 0)
    {
        return -1;
    }

    // If no data available yet, block and wait for notify
    if (!recv_buffer)
    {
        waiting_task = xTaskGetCurrentTaskHandle();
        BaseType_t result = ulTaskNotifyTakeIndexed(NotifyRecv, pdTRUE, pdMS_TO_TICKS(timeout_ms));
        if (result == 0 || !recv_buffer)
            return 0; // timeout or nothing delivered
    }

    if (!recv_buffer)
    {
        return 0; // Nothing was delivered even after wakeup
    }

    // Copy as much as we can from the pbuf to the buffer
    size_t available = recv_buffer->tot_len - recv_offset;
    size_t to_copy = (size < available) ? size : available;
    pbuf_copy_partial(recv_buffer, buffer, to_copy, recv_offset);
    recv_offset += to_copy;

    // If we've consumed the full pbuf, release it
    if (recv_offset >= recv_buffer->tot_len)
    {
        pbuf_free(recv_buffer);
        recv_buffer = nullptr;
        recv_offset = 0;
    }
    return static_cast<int>(to_copy);
#else
    printf("[Tcp] TLS not enabled in this build\n");
    return -1;
#endif
}

int Tcp::close()
{
    int result = 0;
    #if PICO_TCP_ENABLE_TLS
    if (use_tls && tls_pcb)
    {
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
    }
    else 
    #endif
    if (sockfd >= 0)
    {
        result = lwip_close(sockfd);
        sockfd = -1;
    }
    recv_offset = 0;
    waiting_task = nullptr;
    if (recv_buffer)
    {
        pbuf_free(recv_buffer);
        recv_buffer = nullptr;
    }

    connected = false;
    return result;
}

err_t Tcp::acceptCallback(void *arg, struct altcp_pcb *new_conn, err_t err)
{
    auto *self = static_cast<Tcp *>(arg);

    if (err != ERR_OK || !new_conn || !self)
    {
        return ERR_VAL;
    }

    self->pending_client = new_conn;

    if (self->waiting_task)
    {
        xTaskNotifyGiveIndexed(self->waiting_task, NotifyAccept); // Notify waiting task
        self->waiting_task = nullptr;
    }

    return ERR_OK;
}

Tcp *Tcp::accept()
{
    if (use_tls)
    {
        #if PICO_TCP_ENABLE_TLS
        pending_client = nullptr;
        waiting_task = xTaskGetCurrentTaskHandle();
        ulTaskNotifyTakeIndexed(NotifyAccept, pdTRUE, portMAX_DELAY);
        waiting_task = nullptr;

        if (pending_client)
        {
            Tcp *client = new Tcp();
            client->tls_pcb = pending_client;
            client->use_tls = true;
            client->connected = true;
            pending_client = nullptr;
            return client;
        }
        return nullptr;
        #else
        printf("[Tcp] TLS not enabled in this build\n");
        return nullptr;
        #endif
    }

    // For plain TCP, use lwIP accept directly
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = lwip_accept(sockfd, reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);
    TRACE("[Tcp] Accept returned new socket: %d\n", client_fd);

    if (client_fd >= 0)
    {
        // Set receive timeout
        struct timeval recv_timeout = {.tv_sec = 0, .tv_usec = 100000};
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));

        // Optional: Force-close on disconnect (RST on close)
        struct linger so_linger = {.l_onoff = 1, .l_linger = 0};
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

        Tcp *client = new Tcp(client_fd);
        return client;
    }

    printf("[Tcp] Accept timeout, no client.\n");
    return nullptr;
}

bool Tcp::bindAndListen(int port)
{
    #if PICO_TCP_ENABLE_TLS
    return server_tls_config ? bindAndListenTls(port) : bindAndListenPlain(port);
    #else
    return bindAndListenPlain(port);
    #endif
}

#include <fcntl.h> // needed for O_NONBLOCK
bool Tcp::bindAndListenPlain(int port)
{
    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[Tcp] Failed to create socket\n");
        return false;
    }

    int opt = 1;
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    if (lwip_bind(sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        printf("[Tcp] Failed to bind socket\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    if (lwip_listen(sockfd, 10) < 0)
    {
        printf("[Tcp] Failed to listen on socket\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    TRACE("[Tcp] Listening on port: %d, (socket: %d)\n", port, sockfd);

    use_tls = false;
    connected = true;
    return true;
}
#if PICO_TCP_ENABLE_TLS
bool Tcp::bindAndListenTls(int port)
{
    if (!server_tls_config)
    {
        printf("[Tcp] TLS config not set for server\n");
        return false;
    }

    tls_pcb = altcp_tls_new(server_tls_config, IPADDR_TYPE_V4);
    if (!tls_pcb)
    {
        printf("[Tcp] Failed to create TLS PCB\n");
        return false;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    err_t err = altcp_bind(tls_pcb, reinterpret_cast<ip_addr_t *>(&addr.sin_addr), port);
    if (err != ERR_OK)
    {
        printf("[Tcp] altcp_bind failed: %d\n", err);
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
        return false;
    }

    altcp_accept(tls_pcb, Tcp::acceptCallback);
    use_tls = true;
    connected = true;
    return true;
}
#endif
