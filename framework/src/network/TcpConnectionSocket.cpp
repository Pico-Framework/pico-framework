#include "TcpConnectionSocket.h"

#include <lwip/dns.h>
#include <lwip/altcp_tls.h>
#include <lwip/api.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <cstring>
#include <cstdio>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "lwip_dns_resolver.h"

TcpConnectionSocket::TcpConnectionSocket()
    : sockfd(-1), connected(false), use_tls(false),
      tls_config(nullptr), server_tls_config(nullptr),
      tls_pcb(nullptr) {}

TcpConnectionSocket::TcpConnectionSocket(int fd)
    : sockfd(fd), connected(true), use_tls(false),
      tls_config(nullptr), server_tls_config(nullptr),
      tls_pcb(nullptr) {}

TcpConnectionSocket::~TcpConnectionSocket() {
    close();
}

TcpConnectionSocket::TcpConnectionSocket(TcpConnectionSocket&& other) noexcept {
    *this = std::move(other);
}

TcpConnectionSocket& TcpConnectionSocket::operator=(TcpConnectionSocket&& other) noexcept {
    if (this != &other) {
        sockfd          = other.sockfd;
        tls_pcb         = other.tls_pcb;
        tls_config      = other.tls_config;
        server_tls_config = other.server_tls_config;
        connected       = other.connected;
        use_tls         = other.use_tls;
        recv_buffer     = other.recv_buffer;

        other.sockfd      = -1;
        other.tls_pcb     = nullptr;
        other.tls_config  = nullptr;
        other.server_tls_config = nullptr;
        other.recv_buffer = nullptr;
    }
    return *this;
}

void TcpConnectionSocket::setRootCACertificate(const std::string& pem) {
    root_ca_cert = pem;
}

void TcpConnectionSocket::setServerTlsConfig(const std::string& cert, const std::string& key) {
    server_tls_cert = cert;
    server_tls_key = key;

    server_tls_config = altcp_tls_create_config_server_privkey_cert(
        reinterpret_cast<const uint8_t*>(server_tls_key.c_str()), server_tls_key.size(),
        reinterpret_cast<const uint8_t*>(server_tls_cert.c_str()), server_tls_cert.size(),
        nullptr, 0 // no passphrase
    );
    if (!server_tls_config) {
        printf("[TcpConnectionSocket] Failed to create TLS server config\n");
    }
}

bool TcpConnectionSocket::connect(const char* host, int port) {
    ip_addr_t ip;
    if (!resolveHostnameBlocking(host, &ip)) {
        printf("[TcpConnectionSocket] DNS resolution failed for %s\n", host);
        return false;
    }

    return use_tls ? connectTls(ip, port) : connectPlain(ip, port);
}

bool TcpConnectionSocket::connectPlain(const ip_addr_t& ip, int port) {
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = ip.addr;

    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[TcpConnectionSocket] Failed to create socket\n");
        return false;
    }

    if (lwip_connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        printf("[TcpConnectionSocket] Failed to connect to server\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    connected = true;
    use_tls = false;
    return true;
}

bool TcpConnectionSocket::connectTls(const ip_addr_t& ip, int port) {
    if (!tls_config) {
        tls_config = altcp_tls_create_config_client(
            reinterpret_cast<const uint8_t*>(root_ca_cert.c_str()),
            root_ca_cert.size());
        if (!tls_config) {
            printf("[TcpConnectionSocket] TLS config creation failed\n");
            return false;
        }
    }

    tls_pcb = altcp_tls_new(tls_config, IP_GET_TYPE(&ip));
    if (!tls_pcb) {
        printf("[TcpConnectionSocket] Failed to create TLS connection\n");
        return false;
    }

    // Register recv callback *before* connect
    altcp_arg(tls_pcb, this);  // Set the context for the callbacks
    altcp_recv(tls_pcb, tlsRecvCallback);  // Register callback

    err_t err = altcp_connect(tls_pcb, &ip, port, nullptr);
    if (err != ERR_OK) {
        printf("[TcpConnectionSocket] altcp_connect failed: %d\n", err);
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
        return false;
    }

    // Wait briefly for the handshake — altcp doesn't expose state.
    vTaskDelay(pdMS_TO_TICKS(200));

    connected = true;
    use_tls = true;
    return true;
}


int TcpConnectionSocket::send(const char* buffer, size_t size) {
    if (use_tls && tls_pcb) {
        err_t err = altcp_write(tls_pcb, buffer, size, TCP_WRITE_FLAG_COPY);
        if (err == ERR_OK) {
            return altcp_output(tls_pcb) == ERR_OK ? static_cast<int>(size) : -1;
        }
        return -1;
    } else if (sockfd >= 0) {
        return lwip_send(sockfd, buffer, size, 0);
    }
    return -1;
}


err_t TcpConnectionSocket::tlsRecvCallback(void* arg, struct altcp_pcb* conn, struct pbuf* p, err_t err) {
    auto* self = static_cast<TcpConnectionSocket*>(arg);
    if (!self) return ERR_VAL;

    if (!p) {
        // Remote closed connection
        if (self->recv_buffer) {
            pbuf_free(self->recv_buffer);
            self->recv_buffer = nullptr;
        }
        self->recv_offset = 0;
        return ERR_OK;
    }

    // Drop if already have a pbuf (single-buffer policy for now)
    if (self->recv_buffer) {
        pbuf_free(p);  // Drop additional pbuf to prevent overflow
        return ERR_OK;
    }

    self->recv_buffer = p;
    self->recv_offset = 0;

    // Notify any task waiting in recv()
    if (self->waiting_task) {
        xTaskNotifyGiveIndexed(self->waiting_task, NotifyRecv);
        self->waiting_task = nullptr;
    }

    return ERR_OK;
}

int TcpConnectionSocket::recv(char* buffer, size_t size) {
    if (!use_tls) {
        return lwip_recv(sockfd, buffer, size, 0);
    }

    if (!tls_pcb || !buffer || size == 0) {
        return -1;
    }

    // If no data available yet, block and wait for notify
    if (!recv_buffer) {
        waiting_task = xTaskGetCurrentTaskHandle();
        ulTaskNotifyTakeIndexed(NotifyRecv, pdTRUE, portMAX_DELAY); // Wait for data
    }

    if (!recv_buffer) {
        return 0;  // Nothing was delivered even after wakeup
    }

    // Copy as much as we can from the pbuf to the buffer
    size_t available = recv_buffer->tot_len - recv_offset;
    size_t to_copy = (size < available) ? size : available;
    pbuf_copy_partial(recv_buffer, buffer, to_copy, recv_offset);
    recv_offset += to_copy;

    // If we've consumed the full pbuf, release it
    if (recv_offset >= recv_buffer->tot_len) {
        pbuf_free(recv_buffer);
        recv_buffer = nullptr;
        recv_offset = 0;
    }
    return static_cast<int>(to_copy);
}

int TcpConnectionSocket::close() {
    int result = 0;
    if (use_tls && tls_pcb) {
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
    } else if (sockfd >= 0) {
        result = lwip_close(sockfd);
        sockfd = -1;
    }
    recv_offset = 0;
    waiting_task = nullptr;
    if (recv_buffer) {
        pbuf_free(recv_buffer);
        recv_buffer = nullptr;
    }

    connected = false;
    return result;
}

err_t TcpConnectionSocket::acceptCallback(void* arg, struct altcp_pcb* new_conn, err_t err) {
    auto* self = static_cast<TcpConnectionSocket*>(arg);

    if (err != ERR_OK || !new_conn || !self) {
        return ERR_VAL;
    }

    self->pending_client = new_conn;

    if (self->waiting_task) {
        xTaskNotifyGiveIndexed(self->waiting_task, NotifyAccept);   // Notify waiting task
        self->waiting_task = nullptr;
    }

    return ERR_OK;
}

TcpConnectionSocket TcpConnectionSocket::accept() {
    if (use_tls) {
        // TLS-enabled accept: wait for notification
        pending_client = nullptr;
        waiting_task = xTaskGetCurrentTaskHandle();

        ulTaskNotifyTakeIndexed(NotifyAccept, pdTRUE, portMAX_DELAY); // Wait for accept

        waiting_task = nullptr;

        if (pending_client) {
            TcpConnectionSocket client;
            client.tls_pcb = pending_client;
            client.use_tls = true;
            client.connected = true;
            pending_client = nullptr;
            return client;
        }

        // No client was accepted — return invalid
        return TcpConnectionSocket();
    }

    // Plain TCP accept
    struct sockaddr_in client_addr {};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = lwip_accept(sockfd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if (client_fd < 0) {
        printf("[TcpConnectionSocket] lwip_accept failed\n");
        return TcpConnectionSocket();  // Invalid
    }

    return TcpConnectionSocket(client_fd);
}

bool TcpConnectionSocket::bindAndListen(int port) {
    return server_tls_config ? bindAndListenTls(port) : bindAndListenPlain(port);
}

bool TcpConnectionSocket::bindAndListenPlain(int port) {
    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[TcpConnectionSocket] Failed to create socket\n");
        return false;
    }

    int opt = 1;
    lwip_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    if (lwip_bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        printf("[TcpConnectionSocket] Failed to bind socket\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    if (lwip_listen(sockfd, 5) < 0) {
        printf("[TcpConnectionSocket] Failed to listen on socket\n");
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    use_tls = false;
    connected = true;
    return true;
}

bool TcpConnectionSocket::bindAndListenTls(int port) {
    if (!server_tls_config) {
        printf("[TcpConnectionSocket] TLS config not set for server\n");
        return false;
    }

    tls_pcb = altcp_tls_new(server_tls_config, IPADDR_TYPE_V4);
    if (!tls_pcb) {
        printf("[TcpConnectionSocket] Failed to create TLS PCB\n");
        return false;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    err_t err = altcp_bind(tls_pcb, reinterpret_cast<ip_addr_t*>(&addr.sin_addr), port);
    if (err != ERR_OK) {
        printf("[TcpConnectionSocket] altcp_bind failed: %d\n", err);
        altcp_close(tls_pcb);
        tls_pcb = nullptr;
        return false;
    }

    altcp_accept(tls_pcb, TcpConnectionSocket::acceptCallback);
    use_tls = true;
    connected = true;
    return true;
}

