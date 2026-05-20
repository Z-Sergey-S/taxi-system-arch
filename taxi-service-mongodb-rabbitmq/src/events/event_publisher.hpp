#pragma once

#include <memory>
#include <string>
#include "event_types.hpp"

namespace events {

class EventPublisher {
public:
    static EventPublisher& GetInstance();
    
    void Initialize(const std::string& exchange_name);
    
    void Publish(const BaseEvent& event);
    
    bool IsInitialized() const { return initialized_; }

private:
    EventPublisher() = default;
    ~EventPublisher() = default;
    
    std::string exchange_name_;
    bool initialized_{false};
    
    void EnsureInitialized();
    std::string GetRoutingKey(EventType type);
};

} // namespace events