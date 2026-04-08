#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <userver/storages/postgres/cluster.hpp>
#include "../models/user.hpp"

namespace db {

class UserRepository {
public:
    explicit UserRepository(const userver::storages::postgres::ClusterPtr& pg_cluster);
    
    // Создание пользователя
    models::User CreateUser(const models::User::CreateRequest& request);
    
    // Поиск по логину
    std::optional<models::User> FindUserByLogin(const std::string& login);
    
    // Поиск по маске имени/фамилии
    std::vector<models::User> FindUsersByNameMask(const std::string& first_name, const std::string& last_name);
    
    // Поиск по ID
    std::optional<models::User> FindUserById(const std::string& user_id);
    
    // Проверка пароля
    bool ValidatePassword(const std::string& login, const std::string& password);
    
    // Аутентификация
    std::optional<models::User> Authenticate(const std::string& login, const std::string& password);
    
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
    
    std::string HashPassword(const std::string& password);
};

} // namespace db