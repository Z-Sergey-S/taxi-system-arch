#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <optional>

namespace events {

enum class EventType {
    UserCreated,
    RideCreated,
    RideAccepted,
    RideCompleted
};

class BaseEvent {
public:
    BaseEvent(EventType type, const std::string& event_id, std::chrono::system_clock::time_point timestamp);
    virtual ~BaseEvent() = default;

    EventType GetType() const;
    const std::string& GetEventId() const;
    std::chrono::system_clock::time_point GetTimestamp() const;

    friend std::unique_ptr<BaseEvent> DeserializeEvent(const std::string& json);

protected:
    EventType type_;
    std::string event_id_;
    std::chrono::system_clock::time_point timestamp_;
};

struct UserCreatedEvent : public BaseEvent {
    UserCreatedEvent();
    UserCreatedEvent(
        const std::string& user_id,
        const std::string& login,
        const std::string& first_name,
        const std::string& last_name,
        const std::string& email
    );

    std::string user_id;
    std::string login;
    std::string first_name;
    std::string last_name;
    std::string email;
};

struct RideCreatedEvent : public BaseEvent {
    RideCreatedEvent();
    RideCreatedEvent(
        const std::string& ride_id,
        const std::string& user_id,
        const std::string& start_address,
        const std::string& end_address,
        double price
    );

    std::string ride_id;
    std::string user_id;
    std::string start_address;
    std::string end_address;
    double price;
};

struct RideAcceptedEvent : public BaseEvent {
    RideAcceptedEvent();
    RideAcceptedEvent(
        const std::string& ride_id,
        const std::string& driver_id,
        const std::string& driver_name,
        const std::string& car_number
    );

    std::string ride_id;
    std::string driver_id;
    std::string driver_name;
    std::string car_number;
};

struct RideCompletedEvent : public BaseEvent {
    RideCompletedEvent();
    RideCompletedEvent(
        const std::string& ride_id,
        double final_price,
        const std::string& completion_time
    );

    std::string ride_id;
    double final_price;
    std::string completion_time;
};

std::string SerializeEvent(const BaseEvent& event);
std::unique_ptr<BaseEvent> DeserializeEvent(const std::string& json);

} // namespace events