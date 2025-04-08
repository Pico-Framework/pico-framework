/**
 * @file TcpConnectionSocket.h
 * @brief General-purpose TCP socket abstraction with optional TLS support for both client and server use.
 * @author Ian Archbell
 * @date 2023-03-15
 * @license MIT License
 * @copyright Copyright (c) 2023, Ian Archbell
 */
#pragma once
#include <FreeRTOS.h>
#include <queue.h>
#include <string>
#include <vector>
#include <cstdint>
#include <lwip/ip_addr.h>
#include <lwip/sockets.h>
#include <lwip/altcp.h>
#include <lwip/altcp_tls.h>


enum class SocketEvent : uint8_t {
    Connect = 0,
    Recv    = 1,
    Sent    = 2,
    Error   = 3
};

enum NotifyIndex {
    NotifyRecv = 0,
    NotifyAccept = 1,
    NotifyConnect = 2,
};

/**
 * @brief General-purpose TCP socket wrapper with optional TLS support via mbedTLS (altcp).
 */
class TcpConnectionSocket {
public:
    TcpConnectionSocket();
    explicit TcpConnectionSocket(int sockfd); ///< For accepted sockets
    ~TcpConnectionSocket();

    // Disable copy
    TcpConnectionSocket(const TcpConnectionSocket&) = delete;
    TcpConnectionSocket& operator=(const TcpConnectionSocket&) = delete;

    // Enable move
    TcpConnectionSocket(TcpConnectionSocket&& other) noexcept;
    TcpConnectionSocket& operator=(TcpConnectionSocket&& other) noexcept;

    /**
     * @brief Set the Root CA certificate to be used for client TLS connections (PEM format).
     */
    void setRootCACertificate(const std::string& pem);

    /**
     * @brief Set the certificate and key to use for server-side TLS (PEM format).
     */
    void setServerTlsConfig(const std::string& certPem, const std::string& keyPem);

    /**
     * @brief Connect to a remote host.
     * @param host Hostname or IP address.
     * @param port Port number.
     * @return true on success, false on failure.
     */
    bool connect(const char* host, int port);
    bool connectTls(const char* host, int port);
    static err_t onConnected(void* arg, struct altcp_pcb* conn, err_t err);
    static void onError(void* arg, err_t err);

    /**
     * @brief Send data over the connection.
     */
    int send(const char* buffer, size_t size);

    /**
     * @brief Receive data from the connection.
     */
    int recv(char* buffer, size_t size);

    /**
     * @brief Close the connection and free resources.
     */
    int close();

    /**
     * @brief Bind and listen on a port for incoming connections (for server use).
     */
    bool bindAndListen(int port);
    bool bindAndListenPlain(int port);
    bool bindAndListenTls(int port);

    /**
     * @brief Accept a new incoming connection (for server use).
     * @return TcpConnectionSocket for the accepted client.
     */
    TcpConnectionSocket accept();

    /**
     * @brief Check if the socket is valid.
     */
    bool isValid() const { return sockfd >= 0 || tls_pcb != nullptr; }

    /**
     * @brief Check if the socket is connected.
     */
    bool isConnected() const { return connected; }

    /**
     * @brief Get the raw socket file descriptor (may be -1 for TLS-only connection).
     */
    int getSocketFd() const;

    void setHostname(const char* name) {
        if (name) {
            strncpy(hostname, name, sizeof(hostname) - 1);
            hostname[sizeof(hostname) - 1] = '\0';
        }
    }
    const char* getHostname() const { return hostname; }

private:
    // Internal helpers
    bool connectPlain(const ip_addr_t& ip, int port);
    bool connectTls(const ip_addr_t& ip, int port);

    static err_t tlsRecvCallback(void* arg, struct altcp_pcb* conn, struct pbuf* p, err_t err);
    static err_t acceptCallback(void* arg, struct altcp_pcb* new_conn, err_t err);

    int sockfd = -1;
    bool connected = false;
    bool use_tls = false;
    bool is_server_socket = false;
    int connectResult = ERR_OK;

    std::string root_ca_cert;

    // TLS state (client)
    struct altcp_tls_config* tls_config = nullptr;
    struct altcp_pcb* tls_pcb = nullptr;
    struct pbuf* recv_buffer = nullptr; ///< Buffer for TLS receive
    size_t recv_offset = 0;

    // TLS server config
    std::string server_tls_cert;
    std::string server_tls_key;
    struct altcp_tls_config* server_tls_config = nullptr;

    // buffering receive pbufs using callabck and queue
    static constexpr size_t MAX_TLS_SEGMENT_SIZE = 1460; // Typical MSS size

    TaskHandle_t connectingTask = nullptr; ///< Task handle for async operations
    TaskHandle_t waiting_task = nullptr; ///< Task handle for async operations
    altcp_pcb* pending_client = nullptr; ///< For TLS: set by acceptCallback

    char hostname[64] = {0}; ///< Hostname for TLS connections

};
