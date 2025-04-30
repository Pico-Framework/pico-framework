#pragma once
#include <unordered_map>
#include <cstdint>
#include <type_traits>
#include <string_view>
#include <FreeRTOS.h>
#include <semphr.h>

class AppContext {
public:
    static AppContext& getInstance();

    /**
     * @brief Register a service of type T in the application context.
     * This allows other parts of the application to retrieve the service using getService<T>().
     * @param service Pointer to the service instance to register.
     * @tparam T The type of the service to register. Must be a complete type.
     * @note The service must outlive the AppContext instance to avoid dangling pointers.
     * @warning This does not check for duplicate registrations. If a service of the same type is registered again,
     * it will overwrite the previous one without warning.
     * Ensure that services are registered only once or handle duplicates appropriately.
     * @example
     * AppContext::getInstance().registerService<MyService>(new MyService());
     * System services are static instances that are registered in the AppContext.
     */
    template<typename T>
    void registerService(T* service) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        services[getTypeKey<T>()] = reinterpret_cast<void*>(service);
        xSemaphoreGive(mutex);
    }

    /**
     * @brief Retrieve a service of type T from the application context.
     * @tparam T The type of the service to retrieve. Must be a complete type.
     * @return Pointer to the service instance if found, nullptr otherwise.
     * @note If the service is not registered, this will return nullptr.
     * @example
     * MyService* myService = AppContext::getInstance().getService<MyService>();
     * This allows components to access shared services without needing to know about their concrete types.
     * @warning If the service is not registered, this will return nullptr.
     * Ensure to check for nullptr before using the returned pointer to avoid dereferencing a null pointer.
     */
    template<typename T>
    T* getService() const {
        xSemaphoreTake(mutex, portMAX_DELAY);
        auto it = services.find(getTypeKey<T>());
        xSemaphoreGive(mutex);
        if (it != services.end()) {
            return reinterpret_cast<T*>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Get a service of type T from the application context.
     * This is a convenience method that allows you to get a service without needing to call getService<T>() explicitly.
     * @tparam T The type of the service to retrieve. Must be a complete type.
     * @return Pointer to the service instance if found, nullptr otherwise.
     * @note If the service is not registered, this will return nullptr.
     */
    template<typename T>
    static T* get() {
        return getInstance().getService<T>();
    }

    /**
     * @brief Initialize the framework services.
     * This method sets up all the core services required by the framework.
     * It should be called once at the start of the application to ensure all services are available.
     * @note This method is responsible for registering the core services like TimeManager, EventManager, etc.
     * It is typically called in the main application entry point.
     * @example
     * AppContext::getInstance().initFrameworkServices();
     * This initializes the framework services and makes them available for use throughout the application.
     */
    void initFrameworkServices();

private:
    AppContext() = default;
    std::unordered_map<std::uintptr_t, void*> services;

    static SemaphoreHandle_t mutex;

    template<typename T>
    static constexpr std::uintptr_t getTypeKey() {
        return reinterpret_cast<std::uintptr_t>(&getTypeKey<T>); // unique per T
    }
};
