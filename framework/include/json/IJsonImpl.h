#pragma once
#include <string>
#include <memory>
#include <initializer_list>
#include <map>
#include <vector>

namespace Framework {

class json;  // forward declaration

class IJsonImpl {
public:
    virtual ~IJsonImpl() = default;

    // Cloning
    virtual std::shared_ptr<IJsonImpl> clone() const = 0;

    // Serialization
    virtual std::string dump(int indent = -1) const = 0;

    // Access / mutation
    virtual json at(const std::string& key) const = 0;
    virtual json& refAt(const std::string& key) = 0;

    // Object/array construction
    virtual bool is_object() const = 0;
    virtual bool is_array() const = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;

    // Primitive getters
    virtual bool is_string() const = 0;
    virtual bool is_boolean() const = 0;
    virtual bool is_number() const = 0;
    virtual bool is_null() const = 0;

    template<typename T> T get() const;
    template<typename T> void get_to(T&) const;

    // Key-value helper
    virtual bool contains(const std::string& key) const = 0;
};

}  // namespace Framework
