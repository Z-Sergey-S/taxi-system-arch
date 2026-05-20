#include "event_publisher.hpp"
#include <userver/logging/log.hpp>
#include <chrono>

namespace events {

EventPublisher& EventPublisher::GetInstance() {
    static EventPublisher instance;
    return instance;
}

void EventPublisher::Initialize(const std::string& exchange_name) {
    exchange_name_ = exchange_name;
    initialized_ = true;
    LOG_INFO() << "EventPublisher initialized with exchange: " << exchange_name;
}

void EventPublisher::EnsureInitialized() {
    if (!initialized_) {
        throw std::runtime_error("EventPublisher not initialized. Call Initialize() first.");
    }
}

std::string EventPublisher::GetRoutingKey(EventType type) {
    switch (type) {
        case EventType::UserCreated:
            return "user.created";
        case EventType::RideCreated:
            return "ride.created";
        case EventType::RideAccepted:
            return "ride.accepted";
        case EventType::RideCompleted:
            return "ride.completed";
        default:
            return "unknown";
    }
}

void EventPublisher::Publish(const BaseEvent& event) {
    EnsureInitialized();
    
    std::string routing_key = GetRoutingKey(event.GetType());
    std::string message = SerializeEvent(event);
    
    LOG_INFO() << "Publishing event: " << routing_key 
               << ", event_id: " << event.GetEventId()
               << ", message: " << message;
}

} // namespace events