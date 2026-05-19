#include "event_producer.hpp"
#include <userver/urabbitmq/component.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/deadline.hpp>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>

namespace events {

static std::string GenerateUuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ss << '-';
        } else if (i == 14) {
            ss << dis2(gen);
        } else {
            ss << dis(gen);
        }
    }
    return ss.str();
}

EventProducer::EventProducer(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      rabbit_client_(context.FindComponent<userver::components::RabbitMQ>("rabbit-producer").GetClient()) {
    // Объявляем exchange при старте
    rabbit_client_->DeclareExchange("taxi.events", userver::urabbitmq::Exchange::Type::kTopic);
}

void EventProducer::PublishEvent(const std::string& routingKey, const std::string& eventType,
                                 const userver::formats::json::Value& data) {
    using namespace userver::formats::json;
    
    ValueBuilder builder;
    builder["eventId"] = GenerateUuid();
    builder["eventType"] = eventType;
    builder["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    builder["data"] = data;

    std::string message = ToString(builder.ExtractValue());
    
    // Публикуем с указанием exchange, routing key и сообщения
    rabbit_client_->Publish("taxi.events", routingKey, message,
                            userver::engine::Deadline::FromDuration(std::chrono::seconds(5)));
}

void EventProducer::PublishUserCreated(const std::string& userId, const std::string& login,
                                       const std::string& firstName, const std::string& lastName,
                                       const std::string& email) {
    using namespace userver::formats::json;
    ValueBuilder data;
    data["userId"] = userId;
    data["login"] = login;
    data["firstName"] = firstName;
    data["lastName"] = lastName;
    data["email"] = email;
    PublishEvent("user.created", "UserCreated", data.ExtractValue());
}

void EventProducer::PublishDriverRegistered(const std::string& driverId, const std::string& login,
                                            const std::string& carModel, const std::string& carNumber) {
    using namespace userver::formats::json;
    ValueBuilder data;
    data["driverId"] = driverId;
    data["login"] = login;
    data["carModel"] = carModel;
    data["carNumber"] = carNumber;
    PublishEvent("driver.registered", "DriverRegistered", data.ExtractValue());
}

void EventProducer::PublishRideCreated(const std::string& rideId, const std::string& userId,
                                       const std::string& startAddress, const std::string& endAddress,
                                       double price, const std::string& createdAt) {
    using namespace userver::formats::json;
    ValueBuilder data;
    data["rideId"] = rideId;
    data["userId"] = userId;
    data["startAddress"] = startAddress;
    data["endAddress"] = endAddress;
    data["price"] = price;
    data["createdAt"] = createdAt;
    PublishEvent("ride.created", "RideCreated", data.ExtractValue());
}

void EventProducer::PublishRideAccepted(const std::string& rideId, const std::string& driverId,
                                        const std::string& acceptedAt) {
    using namespace userver::formats::json;
    ValueBuilder data;
    data["rideId"] = rideId;
    data["driverId"] = driverId;
    data["acceptedAt"] = acceptedAt;
    PublishEvent("ride.accepted", "RideAccepted", data.ExtractValue());
}

void EventProducer::PublishRideCompleted(const std::string& rideId, const std::string& driverId,
                                         const std::string& completedAt, double finalPrice) {
    using namespace userver::formats::json;
    ValueBuilder data;
    data["rideId"] = rideId;
    data["driverId"] = driverId;
    data["completedAt"] = completedAt;
    data["finalPrice"] = finalPrice;
    PublishEvent("ride.completed", "RideCompleted", data.ExtractValue());
}

} // namespace events