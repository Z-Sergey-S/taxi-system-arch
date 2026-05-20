#include "accept_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/auth_middleware.hpp"
#include "../cache/redis_client.hpp"
#include "../cache/cache_keys.hpp"
#include "../rate_limit/rate_limit_middleware.hpp"
#include "../events/event_publisher.hpp"
#include "../events/event_types.hpp"
#include <userver/logging/log.hpp>

namespace handlers {

AcceptRideHandler::AcceptRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string AcceptRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {

    const auto& ride_id = request.GetPathArg("ride_id");

    // Rate limiting
    int remaining = 0, limit = 0, reset = 0;
    std::string user_key = rate_limit::RateLimitMiddleware::GetUserKey(request);
    if (!rate_limit::RateLimitMiddleware::CheckLimit(user_key, remaining, limit, reset)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kTooManyRequests);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Too many requests. Rate limit exceeded.";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver not authenticated";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    const auto& driver_id = auth_result->first;

    using userver::formats::bson::MakeDoc;
    auto rides_collection = pool_->GetCollection("rides");
    auto drivers_collection = pool_->GetCollection("drivers");

    auto ride_doc = rides_collection.FindOne(MakeDoc("_id", userver::formats::bson::Oid(ride_id)));
    if (!ride_doc) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Ride not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    auto ride = *ride_doc;
    if (ride["status"].template As<std::string>() != "created") {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Ride is not in created state";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    auto driver_doc = drivers_collection.FindOne(MakeDoc("_id", userver::formats::bson::Oid(driver_id)));
    if (!driver_doc) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    auto driver = *driver_doc;
    if (driver["status"].template As<std::string>() != "free") {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver is not free";
        return userver::formats::json::ToString(error.ExtractValue());
    }

    rides_collection.UpdateOne(
        MakeDoc("_id", userver::formats::bson::Oid(ride_id)),
        MakeDoc("$set", MakeDoc("driver_id", userver::formats::bson::Oid(driver_id),
                                "status", "accepted",
                                "accepted_at", userver::formats::bson::Oid()))
    );

    drivers_collection.UpdateOne(
        MakeDoc("_id", userver::formats::bson::Oid(driver_id)),
        MakeDoc("$set", MakeDoc("status", "busy"))
    );

    // Публикация события RideAccepted
    try {
        std::string driver_name = driver["first_name"].template As<std::string>() + " " + 
                                  driver["last_name"].template As<std::string>();
        std::string car_number = driver["car_number"].template As<std::string>();
        events::RideAcceptedEvent event(ride_id, driver_id, driver_name, car_number);
        auto& publisher = events::EventPublisher::GetInstance();
        if (publisher.IsInitialized()) {
            publisher.Publish(event);
            LOG_INFO() << "RideAccepted event published for ride: " << ride_id;
        } else {
            LOG_WARNING() << "EventPublisher not initialized, skipping RideAccepted event";
        }
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to publish RideAccepted event: " << e.what();
    }

    auto& redis = cache::RedisClient::GetInstance();
    redis.Del(cache::ActiveRidesKey());

    userver::formats::json::ValueBuilder response;
    response["message"] = "Ride accepted successfully";
    response["ride_id"] = ride_id;

    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers