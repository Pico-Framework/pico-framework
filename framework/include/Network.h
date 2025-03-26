#ifndef NETWORK_H
#define NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

class Network {

    public: 
        static void start_wifi();
        static void wifi_deinit();
        static void wifi_scan();
        static void ping_init();
        static int  getLinkStatus();
        static bool isConnected();
        void arch_init();

    private:
        static bool wifiConnected;
};

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H */