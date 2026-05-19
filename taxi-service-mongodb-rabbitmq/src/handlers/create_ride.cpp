#include "create_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/auth_middleware.hpp"
#include "../models/ride.hpp"
#include "../cache/redis_client.hpp"
#include "../cache/cache_keys.hpp"
#include "../rate_limit/rate_limit_middleware.hpp"
#include "../events/event_producer.hpp"
#include <cmath>

namespace handlers {

CreateRideHandler::CreateRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()),
      event_producer_(&context.FindComponent<events::EventProducer>()) {}

double CalculatePrice(const std::string& start_address, const std::string& end_address) {
    double base_price = 50.0;
    double distance_factor = 10.0;
    return std::round((base_price + (start_address.length() + end_address.length()) * distance_factor) * 100) / 100;
}

std::string CreateRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
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
            error["error"] = "Unauthorized";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseRideCreateRequest(request_body);
        
        double price = CalculatePrice(create_request.start_address, create_request.end_address);
        
        using userver::formats::bson::MakeDoc;
        auto rides_collection = pool_->GetCollection("rides");
        
        rides_collection.InsertOne(
            MakeDoc("user_id", userver::formats::bson::Oid(create_request.user_id),
                    "start_address", create_request.start_address,
                    "end_address", create_request.end_address,
                    "status", "created",
                    "price", price,
                    "created_at", userver::formats::bson::Oid())
        );
        
        auto new_ride = rides_collection.FindOne(
            MakeDoc("user_id", userver::formats::bson::Oid(create_request.user_id),
                    "start_address", create_request.start_address,
                    "end_address", create_request.end_address)
        );
        
        std::string ride_id;
        if (new_ride) {
            ride_id = new_ride->operator[]("_id").template As<userver::formats::bson::Oid>().ToString();
        }
        
        models::Ride::Response response;
        response.id = ride_id;
        response.user_id = create_request.user_id;
        response.start_address = create_request.start_address;
        response.end_address = create_request.end_address;
        response.status = "created";
        response.price = price;
        response.created_at = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        
        auto& redis = cache::RedisClient::GetInstance();
        redis.Del(cache::ActiveRidesKey());
        
        event_producer_->PublishRideCreated(ride_id, create_request.user_id,
                                            create_request.start_address,
                                            create_request.end_address,
                                            price, response.created_at);
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(models::Serialize(response));
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers