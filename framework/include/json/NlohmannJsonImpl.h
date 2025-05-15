#pragma once
#include "json/IJsonImpl.h"
#include <nlohmann/json.hpp>

namespace Framework {

class NlohmannJsonImpl : public IJsonImpl {
public:
    NlohmannJsonImpl();
    explicit NlohmannJsonImpl(const nlohmann::json& j);
    explicit NlohmannJsonImpl(nlohmann::json&& j);

    std::shared_ptr<IJsonImpl> clone() const override;

    std::string dump(int indent = -1) const override;

    json at(const std::string& key) const override;
    json& refAt(const std::string& key) override;

    bool is_object() const override;
    bool is_array() const override;
    bool is_null() const override;
    bool is_string() const override;
    bool is_boolean() const override;
    bool is_number() const override;

    size_t size() const override;
    bool empty() const override;

    bool contains(const std::string& key) const override;

    const nlohmann::json& raw() const { return j_; }
    nlohmann::json& raw() { return j_; }

private:
    nlohmann::json j_;
};

}  // namespace Framework
