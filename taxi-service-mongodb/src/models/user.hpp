#pragma once

#include <string>
#include <userver/formats/json.hpp>

namespace models {

struct User {
    std::string id;
    std::string login;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::string password_hash;
    std::string created_at;
    
    struct CreateRequest {
        std::string login;
        std::string password;
        std::string first_name;
        std::string last_name;
        std::string email;
    };
    
    struct Response {
        std::string id;
        std::string login;
        std::string first_name;
        std::string last_name;
        std::string email;
        std::string created_at;
    };
};

// Функции парсинга и сериализации
models::User::CreateRequest ParseCreateRequest(const userver::formats::json::Value& value);
userver::formats::json::Value Serialize(const models::User::Response& user);

} // namespace models

// Определения функций
inline models::User::CreateRequest models::ParseCreateRequest(const userver::formats::json::Value& value) {
    models::User::CreateRequest request;
    request.login = value["login"].As<std::string>();
    request.password = value["password"].As<std::string>();
    request.first_name = value["first_name"].As<std::string>();
    request.last_name = value["last_name"].As<std::string>();
    request.email = value["email"].As<std::string>();
    return request;
}

inline userver::formats::json::Value models::Serialize(const models::User::Response& user) {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = user.id;
    builder["login"] = user.login;
    builder["first_name"] = user.first_name;
    builder["last_name"] = user.last_name;
    builder["email"] = user.email;
    builder["created_at"] = user.created_at;
    return builder.ExtractValue();
}