#pragma once

#include "FrameworkModel.h"

class UserModel : public FrameworkModel {
public:
    explicit UserModel(StorageManager* storage);

    std::string getUsername(const std::string& id) const;
    void setUsername(const std::string& id, const std::string& newName);

protected:
    std::string getIdField() const override { return "id"; }
};
