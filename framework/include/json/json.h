#pragma once
#include <memory>
#include <string>
#include <initializer_list>
#include "NlohmannJsonImpl.h"

namespace Framework {

class IJsonImpl;

class json {
public:
    json();  // default is null
    json(const json&);
    json(json&&) noexcept;
    json& operator=(const json&);
    json& operator=(json&&) noexcept;

    // Primitive constructors
    json(bool b);
    json(int i);
    json(double d);
    json(const char* s);
    json(const std::string& s);

    ~json();

    static json object();
    static json array();

    std::string dump(int indent = -1) const;

    // Access
    json operator[](const std::string& key) const;
    json& operator[](const std::string& key);

    // Utilities
    size_t size() const;
    bool empty() const;

    bool is_null() const;
    bool is_object() const;
    bool is_array() const;
    bool is_string() const;
    bool is_boolean() const;
    bool is_number() const;


    template<typename T>
    T get() const {
        auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
        return backend->raw().get<T>();
    }
    
    template<typename T>
    void get_to(T& out) const {
        auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
        backend->raw().get_to(out);
    }
    
    template<typename T>
    T value(const std::string& key, const T& default_val) const {
        auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
        return backend->raw().value(key, default_val);
    }

    void push_back(const json& value);

    auto begin() const -> nlohmann::json::const_iterator;
    auto end() const -> nlohmann::json::const_iterator;

    nlohmann::json& raw();
    const nlohmann::json& raw() const;

    json(std::initializer_list<std::pair<std::string, json>> init);
    json& operator=(std::initializer_list<std::pair<std::string, json>> init);




private:
    explicit json(std::shared_ptr<IJsonImpl> impl);
    std::shared_ptr<IJsonImpl> impl;

    friend class NlohmannJsonImpl;
};

}  // namespace Framework
