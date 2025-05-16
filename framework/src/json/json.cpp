#include "json/Json.h"
#include "json/NlohmannJsonImpl.h"

namespace Framework {

json::json() : impl(std::make_shared<NlohmannJsonImpl>()) {}
json::json(std::shared_ptr<IJsonImpl> impl_) : impl(std::move(impl_)) {}

json::json(const json& other) : impl(other.impl->clone()) {}
json::json(json&& other) noexcept : impl(std::move(other.impl)) {}

json& json::operator=(const json& other) {
    if (this != &other) impl = other.impl->clone();
    return *this;
}
json& json::operator=(json&& other) noexcept {
    if (this != &other) impl = std::move(other.impl);
    return *this;
}

json::json(bool b)
    : impl(std::make_shared<NlohmannJsonImpl>(nlohmann::json(b))) {}

json::json(int i)
    : impl(std::make_shared<NlohmannJsonImpl>(nlohmann::json(i))) {}

json::json(double d)
    : impl(std::make_shared<NlohmannJsonImpl>(nlohmann::json(d))) {}

json::json(const char* s)
    : impl(std::make_shared<NlohmannJsonImpl>(nlohmann::json(s))) {}

json::json(const std::string& s)
    : impl(std::make_shared<NlohmannJsonImpl>(nlohmann::json(s))) {}

json::~json() = default;

json json::object() {
    return json(std::make_shared<NlohmannJsonImpl>(nlohmann::json::object()));
}

json json::array() {
    return json(std::make_shared<NlohmannJsonImpl>(nlohmann::json::array()));
}

std::string json::dump(int indent) const {
    return impl->dump(indent);
}

json json::operator[](const std::string& key) const {
    return impl->at(key);
}

json& json::operator[](const std::string& key) {
    return impl->refAt(key);  // NOTE: temporary implementation, refine later
}

bool json::is_object() const { return impl->is_object(); }
bool json::is_array() const { return impl->is_array(); }
bool json::is_null() const { return impl->is_null(); }
bool json::is_string() const { return impl->is_string(); }
bool json::is_boolean() const { return impl->is_boolean(); }
bool json::is_number() const { return impl->is_number(); }

size_t json::size() const { return impl->size(); }
bool json::empty() const { return impl->empty(); }

void json::push_back(const json& value) {
    auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
    const auto* valBackend = static_cast<const NlohmannJsonImpl*>(value.impl.get());
    backend->raw().push_back(valBackend->raw());
}

auto json::begin() const -> nlohmann::json::const_iterator {
    auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
    return backend->raw().begin();
}

auto json::end() const -> nlohmann::json::const_iterator {
    auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
    return backend->raw().end();
}

nlohmann::json& json::raw() {
    auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
    return backend->raw();
}

const nlohmann::json& json::raw() const {
    auto* backend = static_cast<NlohmannJsonImpl*>(impl.get());
    return backend->raw();
}

json::json(std::initializer_list<std::pair<std::string, json>> init)
    : impl(std::make_shared<NlohmannJsonImpl>())
{
    auto& j = static_cast<NlohmannJsonImpl&>(*impl).raw();
    for (const auto& [k, v] : init)
        j[k] = v.raw();  // get internal nlohmann::json
}

json& json::operator=(std::initializer_list<std::pair<std::string, json>> init)
{
    auto& j = static_cast<NlohmannJsonImpl&>(*impl).raw();
    j = nlohmann::json::object();  // clear and prepare as object
    for (const auto& [k, v] : init)
        j[k] = v.raw();
    return *this;
}


}  // namespace Framework

