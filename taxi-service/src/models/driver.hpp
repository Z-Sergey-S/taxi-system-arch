#pragma once

#include <string>
#include <userver/formats/json.hpp>

namespace models {

enum class DriverStatus {
    Free,
    Busy,
    Offline
};

struct Driver {
    std::string id;
    std::string first_name;
    std::string last_name;
    std::string car_model;
    std::string car_number;
    DriverStatus status;
    double rating;
    std::string created_at;
    
    struct CreateRequest {
        std::string first_name;
        std::string last_name;
        std::string car_model;
        std::string car_number;
    };
    
    struct Response {
        std::string id;
        std::string first_name;
        std::string last_name;
        std::string car_model;
        std::string car_number;
        std::string status;
        double rating;
        std::string created_at;
    };
};

// Функции парсинга и сериализации
Driver::CreateRequest ParseDriverCreateRequest(const userver::formats::json::Value& value);
std::string DriverStatusToString(DriverStatus status);
userver::formats::json::Value Serialize(const Driver::Response& driver);

} // namespace models

// Определения функций
inline models::Driver::CreateRequest models::ParseDriverCreateRequest(const userver::formats::json::Value& value) {
    models::Driver::CreateRequest request;
    request.first_name = value["first_name"].As<std::string>();
    request.last_name = value["last_name"].As<std::string>();
    request.car_model = value["car_model"].As<std::string>();
    request.car_number = value["car_number"].As<std::string>();
    return request;
}

inline std::string models::DriverStatusToString(models::DriverStatus status) {
    switch (status) {
        case models::DriverStatus::Free: return "free";
        case models::DriverStatus::Busy: return "busy";
        case models::DriverStatus::Offline: return "offline";
    }
    return "unknown";
}

inline userver::formats::json::Value models::Serialize(const models::Driver::Response& driver) {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = driver.id;
    builder["first_name"] = driver.first_name;
    builder["last_name"] = driver.last_name;
    builder["car_model"] = driver.car_model;
    builder["car_number"] = driver.car_number;
    builder["status"] = driver.status;
    builder["rating"] = driver.rating;
    builder["created_at"] = driver.created_at;
    return builder.ExtractValue();
}