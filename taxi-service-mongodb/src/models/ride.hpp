#pragma once

#include <string>
#include <optional>
#include <userver/formats/json.hpp>

namespace models {

enum class RideStatus {
    Created,
    Accepted,
    InProgress,
    Completed,
    Cancelled
};

struct Ride {
    std::string id;
    std::string user_id;
    std::optional<std::string> driver_id;
    std::string start_address;
    std::string end_address;
    RideStatus status;
    double price;
    std::string created_at;
    std::optional<std::string> accepted_at;
    std::optional<std::string> completed_at;
    
    struct CreateRequest {
        std::string user_id;
        std::string start_address;
        std::string end_address;
    };
    
    struct Response {
        std::string id;
        std::string user_id;
        std::optional<std::string> driver_id;
        std::string start_address;
        std::string end_address;
        std::string status;
        double price;
        std::string created_at;
        std::optional<std::string> accepted_at;
        std::optional<std::string> completed_at;
    };
};

// Функции с уникальными именами
Ride::CreateRequest ParseRideCreateRequest(const userver::formats::json::Value& value);
std::string RideStatusToString(RideStatus status);
userver::formats::json::Value Serialize(const Ride::Response& ride);

} // namespace models

inline models::Ride::CreateRequest models::ParseRideCreateRequest(const userver::formats::json::Value& value) {
    models::Ride::CreateRequest request;
    request.user_id = value["user_id"].As<std::string>();
    request.start_address = value["start_address"].As<std::string>();
    request.end_address = value["end_address"].As<std::string>();
    return request;
}

inline std::string models::RideStatusToString(models::RideStatus status) {
    switch (status) {
        case models::RideStatus::Created: return "created";
        case models::RideStatus::Accepted: return "accepted";
        case models::RideStatus::InProgress: return "in_progress";
        case models::RideStatus::Completed: return "completed";
        case models::RideStatus::Cancelled: return "cancelled";
    }
    return "unknown";
}

inline userver::formats::json::Value models::Serialize(const models::Ride::Response& ride) {
    userver::formats::json::ValueBuilder builder;
    builder["id"] = ride.id;
    builder["user_id"] = ride.user_id;
    builder["start_address"] = ride.start_address;
    builder["end_address"] = ride.end_address;
    builder["status"] = ride.status;  
    builder["price"] = ride.price;
    builder["created_at"] = ride.created_at;
    
    if (ride.driver_id) {
        builder["driver_id"] = *ride.driver_id;
    }
    if (ride.accepted_at) {
        builder["accepted_at"] = *ride.accepted_at;
    }
    if (ride.completed_at) {
        builder["completed_at"] = *ride.completed_at;
    }
    
    return builder.ExtractValue();
}