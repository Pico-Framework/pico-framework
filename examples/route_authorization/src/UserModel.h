/**
 * @file UserModel.h
 * @author
 * @brief Model for storing and verifying user credentials
 * @version 0.1
 * @date 2025-05-06
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include "framework/FrameworkModel.h"
#include <string>

/**
 * @brief Represents the persistent model of registered users.
 *
 * Inherits from FrameworkModel and stores usernames with hashed passwords.
 * Data is persisted as JSON in "users.json".
 */
class UserModel : public FrameworkModel
{
public:
    /**
     * @brief Construct a new UserModel instance.
     * Loads data from "users.json".
     */
    UserModel();

    /**
     * @brief Create a new user with a hashed password.
     *
     * @param username Username to register
     * @param passwordHash SHA-256 hash of the password
     * @return true if user was created successfully
     * @return false if user already exists
     */
    bool createUser(const std::string &username, const std::string &passwordHash);

    /**
     * @brief Verify a user's credentials.
     *
     * @param username Username to check
     * @param passwordHash SHA-256 hash to compare
     * @return true if credentials match
     * @return false if user not found or hash doesn't match
     */
    bool verifyUser(const std::string &username, const std::string &passwordHash);

protected:
    std::string getIdField() const override { return "username"; }
};
