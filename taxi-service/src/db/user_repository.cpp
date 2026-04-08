#include "user_repository.hpp"
#include <userver/storages/postgres/io/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/storages/postgres/io/user_types.hpp>
#include <userver/storages/postgres/parameter_store.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace db {

UserRepository::UserRepository(const userver::storages::postgres::ClusterPtr& pg_cluster)
    : pg_cluster_(pg_cluster) {}

std::string UserRepository::HashPassword(const std::string& password) {
    return "hash_" + password;
}

models::User UserRepository::CreateUser(const models::User::CreateRequest& request) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            INSERT INTO taxi_schema.users (login, first_name, last_name, email, password_hash)
            VALUES ($1, $2, $3, $4, $5)
            RETURNING id::text, login, first_name, last_name, email, created_at::text
        )",
        request.login, request.first_name, request.last_name, request.email,
        HashPassword(request.password)
    );
    
    if (result.IsEmpty()) {
        throw std::runtime_error("Failed to create user");
    }
    
    auto row = result.Front();
    models::User user;
    user.id = row["id"].As<std::string>();
    user.login = row["login"].As<std::string>();
    user.first_name = row["first_name"].As<std::string>();
    user.last_name = row["last_name"].As<std::string>();
    user.email = row["email"].As<std::string>();
    user.created_at = row["created_at"].As<std::string>();
    return user;
}

std::optional<models::User> UserRepository::FindUserByLogin(const std::string& login) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, login, first_name, last_name, email, created_at::text, password_hash
            FROM taxi_schema.users
            WHERE login = $1
        )",
        login
    );
    
    if (result.IsEmpty()) {
        return std::nullopt;
    }
    
    auto row = result.Front();
    models::User user;
    user.id = row["id"].As<std::string>();
    user.login = row["login"].As<std::string>();
    user.first_name = row["first_name"].As<std::string>();
    user.last_name = row["last_name"].As<std::string>();
    user.email = row["email"].As<std::string>();
    user.created_at = row["created_at"].As<std::string>();
    user.password_hash = row["password_hash"].As<std::string>();
    
    return user;
}

std::vector<models::User> UserRepository::FindUsersByNameMask(
    const std::string& first_name, const std::string& last_name) {
    
    std::string mask = "%" + first_name + "%";
    std::string mask2 = "%" + last_name + "%";
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, login, first_name, last_name, email, created_at::text
            FROM taxi_schema.users
            WHERE first_name ILIKE $1 AND last_name ILIKE $2
        )",
        mask, mask2
    );
    
    std::vector<models::User> users;
    for (const auto& row : result) {
        models::User user;
        user.id = row["id"].As<std::string>();
        user.login = row["login"].As<std::string>();
        user.first_name = row["first_name"].As<std::string>();
        user.last_name = row["last_name"].As<std::string>();
        user.email = row["email"].As<std::string>();
        user.created_at = row["created_at"].As<std::string>();
        users.push_back(user);
    }
    
    return users;
}

std::optional<models::User> UserRepository::FindUserById(const std::string& user_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, login, first_name, last_name, email, created_at::text
            FROM taxi_schema.users
            WHERE id = $1::uuid
        )",
        user_id
    );
    
    if (result.IsEmpty()) {
        return std::nullopt;
    }
    
    auto row = result.Front();
    models::User user;
    user.id = row["id"].As<std::string>();
    user.login = row["login"].As<std::string>();
    user.first_name = row["first_name"].As<std::string>();
    user.last_name = row["last_name"].As<std::string>();
    user.email = row["email"].As<std::string>();
    user.created_at = row["created_at"].As<std::string>();
    
    return user;
}

bool UserRepository::ValidatePassword(const std::string& login, const std::string& password) {
    auto user = FindUserByLogin(login);
    if (!user) {
        return false;
    }
    return user->password_hash == HashPassword(password);
}

std::optional<models::User> UserRepository::Authenticate(const std::string& login, const std::string& password) {
    if (!ValidatePassword(login, password)) {
        return std::nullopt;
    }
    
    auto user = FindUserByLogin(login);
    if (user) {
        user->password_hash.clear();
    }
    return user;
}

} // namespace db