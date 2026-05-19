#include "get_active_rides.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/auth_middleware.hpp"
#include "../models/ride.hpp"
#include "../cache/redis_client.hpp"
#include "../cache/cache_keys.hpp"
#include "../rate_limit/rate_limit_middleware.hpp"
#include <userver/logging/log.hpp>

namespace handlers {

GetActiveRidesHandler::GetActiveRidesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string GetActiveRidesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Unauthorized";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    // Rate limiting
    int remaining = 0, limit = 0, reset = 0;
    std::string user_key = rate_limit::RateLimitMiddleware::GetUserKey(request);
    
    if (!rate_limit::RateLimitMiddleware::CheckLimit(user_key, remaining, limit, reset)) {
        LOG_WARNING() << "Rate limit EXCEEDED for get_active_rides";
        request.SetResponseStatus(userver::server::http::HttpStatus::kTooManyRequests);
        
        userver::formats::json::ValueBuilder error;
        error["error"] = "Too many requests. Rate limit exceeded.";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    // Проверка кеша
    std::string cache_key = cache::ActiveRidesKey();
    auto& redis = cache::RedisClient::GetInstance();
    auto cached_data = redis.Get(cache_key);
    
    if (cached_data.has_value()) {
        LOG_INFO() << "Cache HIT for active rides";
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return cached_data.value();
    }
    
    LOG_INFO() << "Cache MISS, querying MongoDB for active rides";
    
    using userver::formats::bson::MakeDoc;
    namespace options = userver::storages::mongo::options;
    
    auto rides_collection = pool_->GetCollection("rides");
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    // Получаем активные заказы (status = "created" или "accepted")
    auto cursor = rides_collection.Find(
        MakeDoc("status", MakeDoc("$in", userver::formats::bson::MakeArray("created", "accepted"))),
        options::Sort{std::make_pair("created_at", options::Sort::kAscending)}
    );
    
    for (const auto& doc : cursor) {
        userver::formats::json::ValueBuilder ride_json;
        ride_json["id"] = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["user_id"] = doc["user_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["start_address"] = doc["start_address"].template As<std::string>();
        ride_json["end_address"] = doc["end_address"].template As<std::string>();
        ride_json["status"] = doc["status"].template As<std::string>();
        ride_json["price"] = doc["price"].template As<double>();
        ride_json["created_at"] = std::to_string(
            doc["created_at"].template As<userver::formats::bson::Oid>().GetTimePoint().time_since_epoch().count()
        );
        
        // Проверяем наличие driver_id (может отсутствовать)
        auto driver_id_elem = doc["driver_id"];
        if (!driver_id_elem.IsNull()) {
            ride_json["driver_id"] = driver_id_elem.template As<userver::formats::bson::Oid>().ToString();
        }
        
        response.PushBack(ride_json.ExtractValue());
    }
    
    std::string json_response = userver::formats::json::ToString(response.ExtractValue());
    
    // Кешируем на 10 секунд для активных заказов
    redis.Set(cache_key, json_response, 10);
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return json_response;
}

} // namespace handlers