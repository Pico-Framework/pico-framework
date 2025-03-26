#ifndef NTP_CLIENT_H
#define NTP_CLIENT_H

#include "lwip/opt.h"     // Required before other lwIP headers
#include "lwip/ip_addr.h"
#include "lwip/udp.h"

#include <ctime>
#include <functional>  // ✅ Include functional for std::function

class NTPClient {
    public:
        NTPClient();
        void requestTime();
        static void ntpRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
        void setCallback(const std::function<void(time_t)>& cb);  // ✅ Declare callback setter


    private:
        struct udp_pcb *udp = nullptr;  // ✅ Initialize to nullptr and create later
        ip_addr_t ntp_server;
        static constexpr uint16_t NTP_PORT = 123;
        static constexpr uint32_t NTP_DELTA = 2208988800U;  // Difference between NTP and UNIX epoch
        static constexpr uint8_t NTP_MSG_LEN = 48;

        void processResponse(struct pbuf *p);
        void setSystemTime(time_t epoch);

        std::function<void(time_t)> callback;  // ✅ Store callback function

};

#endif // NTP_CLIENT_H
