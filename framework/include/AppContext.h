#pragma once

#include "FatFsStorageManager.h"
#include "TimeManager.h"
#include "JwtAuthenticator.h"
class AppContext {
public:
    static AppContext& getInstance();

    template<typename T>
    void registerService(T* instance) {
        ServiceHolder<T>::instance = instance;
    }

    template<typename T>
    T* getService() const {
        T* ptr = ServiceHolder<T>::instance;
        if (!ptr) {
            // Optional: handle or assert here
        }
        return ptr;
    }

private:

    void initFrameworkServices();
    AppContext() = default;
    static AppContext* instance;

    template<typename T>
    struct ServiceHolder {
        static T* instance;
    };
};
