#pragma once
#include <unordered_map>
#include <cstdint>
#include <type_traits>
#include <string_view>

class AppContext {
public:
    static AppContext& getInstance();

    template<typename T>
    void registerService(T* service) {
        services[getTypeKey<T>()] = reinterpret_cast<void*>(service);
    }

    template<typename T>
    T* getService() const {
        auto it = services.find(getTypeKey<T>());
        if (it != services.end()) {
            return reinterpret_cast<T*>(it->second);
        }
        return nullptr;
    }
    void initFrameworkServices();

private:
    AppContext() = default;
    std::unordered_map<std::uintptr_t, void*> services;

    template<typename T>
    static constexpr std::uintptr_t getTypeKey() {
        return reinterpret_cast<std::uintptr_t>(&getTypeKey<T>); // unique per T
    }
};
