#include "json/NlohmannJsonImpl.h"
#include "json/Json.h"

namespace Framework {

NlohmannJsonImpl::NlohmannJsonImpl()
    : j_(nullptr) {}

NlohmannJsonImpl::NlohmannJsonImpl(const nlohmann::json& j)
    : j_(j) {}

NlohmannJsonImpl::NlohmannJsonImpl(nlohmann::json&& j)
    : j_(std::move(j)) {}

std::shared_ptr<IJsonImpl> NlohmannJsonImpl::clone() const {
    return std::make_shared<NlohmannJsonImpl>(j_);
}

std::string NlohmannJsonImpl::dump(int indent) const {
    return j_.dump(indent);
}

json NlohmannJsonImpl::at(const std::string& key) const {
    return json(std::make_shared<NlohmannJsonImpl>(j_.at(key)));
}

json& NlohmannJsonImpl::refAt(const std::string& key) {
    // NOTE: this is a placeholder — you can revisit it with a proxy design
    j_[key];  // ensure it exists
    return *(new json(std::make_shared<NlohmannJsonImpl>(j_[key])));
}

bool NlohmannJsonImpl::is_object() const { return j_.is_object(); }
bool NlohmannJsonImpl::is_array() const { return j_.is_array(); }
bool NlohmannJsonImpl::is_null() const { return j_.is_null(); }
bool NlohmannJsonImpl::is_string() const { return j_.is_string(); }
bool NlohmannJsonImpl::is_boolean() const { return j_.is_boolean(); }
bool NlohmannJsonImpl::is_number() const { return j_.is_number(); }

size_t NlohmannJsonImpl::size() const { return j_.size(); }
bool NlohmannJsonImpl::empty() const { return j_.empty(); }

bool NlohmannJsonImpl::contains(const std::string& key) const {
    return j_.contains(key);
}

}  // namespace Framework
