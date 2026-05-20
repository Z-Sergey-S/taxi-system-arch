#include "event_types.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <random>
#include <sstream>

namespace events {

namespace {
    std::string GenerateEventId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                ss << '-';
            } else if (i == 14) {
                ss << '4';
            } else if (i == 19) {
                ss << dis2(gen);
            } else {
                ss << std::hex << dis(gen);
            }
        }
        return ss.str();
    }
}

BaseEvent::BaseEvent(EventType type, const std::string& event_id, std::chrono::system_clock::time_point timestamp)
    : type_(type), event_id_(event_id), timestamp_(timestamp) {}

EventType BaseEvent::GetType() const { return type_; }
const std::string& BaseEvent::GetEventId() const { return event_id_; }
std::chrono::system_clock::time_point BaseEvent::GetTimestamp() const { return timestamp_; }

UserCreatedEvent::UserCreatedEvent()
    : BaseEvent(EventType::UserCreated, GenerateEventId(), std::chrono::system_clock::now()) {}

UserCreatedEvent::UserCreatedEvent(
    const std::string& user_id,
    const std::string& login,
    const std::string& first_name,
    const std::string& last_name,
    const std::string& email
) : BaseEvent(EventType::UserCreated, GenerateEventId(), std::chrono::system_clock::now()),
    user_id(user_id), login(login), first_name(first_name), last_name(last_name), email(email) {}

RideCreatedEvent::RideCreatedEvent()
    : BaseEvent(EventType::RideCreated, GenerateEventId(), std::chrono::system_clock::now()), price(0.0) {}

RideCreatedEvent::RideCreatedEvent(
    const std::string& ride_id,
    const std::string& user_id,
    const std::string& start_address,
    const std::string& end_address,
    double price
) : BaseEvent(EventType::RideCreated, GenerateEventId(), std::chrono::system_clock::now()),
    ride_id(ride_id), user_id(user_id), start_address(start_address), end_address(end_address), price(price) {}

RideAcceptedEvent::RideAcceptedEvent()
    : BaseEvent(EventType::RideAccepted, GenerateEventId(), std::chrono::system_clock::now()) {}

RideAcceptedEvent::RideAcceptedEvent(
    const std::string& ride_id,
    const std::string& driver_id,
    const std::string& driver_name,
    const std::string& car_number
) : BaseEvent(EventType::RideAccepted, GenerateEventId(), std::chrono::system_clock::now()),
    ride_id(ride_id), driver_id(driver_id), driver_name(driver_name), car_number(car_number) {}

RideCompletedEvent::RideCompletedEvent()
    : BaseEvent(EventType::RideCompleted, GenerateEventId(), std::chrono::system_clock::now()), final_price(0.0) {}

RideCompletedEvent::RideCompletedEvent(
    const std::string& ride_id,
    double final_price,
    const std::string& completion_time
) : BaseEvent(EventType::RideCompleted, GenerateEventId(), std::chrono::system_clock::now()),
    ride_id(ride_id), final_price(final_price), completion_time(completion_time) {}

std::string SerializeEvent(const BaseEvent& event) {
    userver::formats::json::ValueBuilder builder;

    builder["event_id"] = event.GetEventId();
    builder["timestamp"] = std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(
            event.GetTimestamp().time_since_epoch()
        ).count()
    );

    switch (event.GetType()) {
        case EventType::UserCreated: {
            const auto& e = static_cast<const UserCreatedEvent&>(event);
            builder["event_type"] = "UserCreated";
            builder["data"]["user_id"] = e.user_id;
            builder["data"]["login"] = e.login;
            builder["data"]["first_name"] = e.first_name;
            builder["data"]["last_name"] = e.last_name;
            builder["data"]["email"] = e.email;
            break;
        }
        case EventType::RideCreated: {
            const auto& e = static_cast<const RideCreatedEvent&>(event);
            builder["event_type"] = "RideCreated";
            builder["data"]["ride_id"] = e.ride_id;
            builder["data"]["user_id"] = e.user_id;
            builder["data"]["start_address"] = e.start_address;
            builder["data"]["end_address"] = e.end_address;
            builder["data"]["price"] = e.price;
            break;
        }
        case EventType::RideAccepted: {
            const auto& e = static_cast<const RideAcceptedEvent&>(event);
            builder["event_type"] = "RideAccepted";
            builder["data"]["ride_id"] = e.ride_id;
            builder["data"]["driver_id"] = e.driver_id;
            builder["data"]["driver_name"] = e.driver_name;
            builder["data"]["car_number"] = e.car_number;
            break;
        }
        case EventType::RideCompleted: {
            const auto& e = static_cast<const RideCompletedEvent&>(event);
            builder["event_type"] = "RideCompleted";
            builder["data"]["ride_id"] = e.ride_id;
            builder["data"]["final_price"] = e.final_price;
            builder["data"]["completion_time"] = e.completion_time;
            break;
        }
    }

    return userver::formats::json::ToString(builder.ExtractValue());
}

std::unique_ptr<BaseEvent> DeserializeEvent(const std::string& json) {
    auto value = userver::formats::json::FromString(json);
    std::string event_type = value["event_type"].As<std::string>();

    if (event_type == "UserCreated") {
        auto event = std::make_unique<UserCreatedEvent>();
        event->event_id_ = value["event_id"].As<std::string>();
        event->user_id = value["data"]["user_id"].As<std::string>();
        event->login = value["data"]["login"].As<std::string>();
        event->first_name = value["data"]["first_name"].As<std::string>();
        event->last_name = value["data"]["last_name"].As<std::string>();
        event->email = value["data"]["email"].As<std::string>();
        return event;
    }
    else if (event_type == "RideCreated") {
        auto event = std::make_unique<RideCreatedEvent>();
        event->event_id_ = value["event_id"].As<std::string>();
        event->ride_id = value["data"]["ride_id"].As<std::string>();
        event->user_id = value["data"]["user_id"].As<std::string>();
        event->start_address = value["data"]["start_address"].As<std::string>();
        event->end_address = value["data"]["end_address"].As<std::string>();
        event->price = value["data"]["price"].As<double>();
        return event;
    }
    else if (event_type == "RideAccepted") {
        auto event = std::make_unique<RideAcceptedEvent>();
        event->event_id_ = value["event_id"].As<std::string>();
        event->ride_id = value["data"]["ride_id"].As<std::string>();
        event->driver_id = value["data"]["driver_id"].As<std::string>();
        event->driver_name = value["data"]["driver_name"].As<std::string>();
        event->car_number = value["data"]["car_number"].As<std::string>();
        return event;
    }
    else if (event_type == "RideCompleted") {
        auto event = std::make_unique<RideCompletedEvent>();
        event->event_id_ = value["event_id"].As<std::string>();
        event->ride_id = value["data"]["ride_id"].As<std::string>();
        event->final_price = value["data"]["final_price"].As<double>();
        event->completion_time = value["data"]["completion_time"].As<std::string>();
        return event;
    }

    return nullptr;
}

} // namespace events