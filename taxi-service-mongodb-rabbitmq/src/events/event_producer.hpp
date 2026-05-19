#pragma once

#include <string>
#include <memory>
#include <userver/components/component_base.hpp>
#include <userver/urabbitmq/client.hpp>
#include <userver/formats/json/value.hpp>

namespace events {

class EventProducer : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "event-producer";

    EventProducer(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);
    
    void PublishUserCreated(const std::string& userId, const std::string& login,
                            const std::string& firstName, const std::string& lastName,
                            const std::string& email);
    
    void PublishDriverRegistered(const std::string& driverId, const std::string& login,
                                 const std::string& carModel, const std::string& carNumber);
    
    void PublishRideCreated(const std::string& rideId, const std::string& userId,
                            const std::string& startAddress, const std::string& endAddress,
                            double price, const std::string& createdAt);
    
    void PublishRideAccepted(const std::string& rideId, const std::string& driverId,
                             const std::string& acceptedAt);
    
    void PublishRideCompleted(const std::string& rideId, const std::string& driverId,
                              const std::string& completedAt, double finalPrice);
    
private:
    void PublishEvent(const std::string& routingKey, const std::string& eventType,
                      const userver::formats::json::Value& data);
    
    std::shared_ptr<userver::urabbitmq::Client> rabbit_client_;
};

} // namespace events