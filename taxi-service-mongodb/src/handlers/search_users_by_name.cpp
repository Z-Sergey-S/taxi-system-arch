#include "search_users_by_name.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/user.hpp"
#include "../cache/redis_client.hpp"
#include "../cache/cache_keys.hpp"
#include "../rate_limit/rate_limit_middleware.hpp"
#include <userver/logging/log.hpp>

namespace handlers {

SearchUsersByNameHandler::SearchUsersByNameHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string SearchUsersByNameHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& first_name = request.GetArg("first_name");
    const auto& last_name = request.GetArg("last_name");
    
    LOG_INFO() << "SearchUsersByName: first_name=" << first_name << ", last_name=" << last_name;
    
    // Rate limiting
    int remaining = 0, limit = 0, reset = 0;
    std::string user_key = rate_limit::RateLimitMiddleware::GetUserKey(request);
    
    if (!rate_limit::RateLimitMiddleware::CheckLimit(user_key, remaining, limit, reset)) {
        LOG_WARNING() << "Rate limit EXCEEDED for user: " << user_key;
        request.SetResponseStatus(userver::server::http::HttpStatus::kTooManyRequests);
        
        userver::formats::json::ValueBuilder error;
        error["error"] = "Too many requests. Rate limit exceeded.";
        error["limit"] = limit;
        error["remaining"] = remaining;
        error["reset_seconds"] = reset;
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    // Проверка кеша
    std::string cache_key = cache::UserSearchKey(first_name, last_name);
    auto& redis = cache::RedisClient::GetInstance();
    auto cached_data = redis.Get(cache_key);
    
    if (cached_data.has_value()) {
        LOG_INFO() << "Cache HIT for search: " << first_name << " " << last_name;
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return cached_data.value();
    }
    
    LOG_INFO() << "Cache MISS, querying MongoDB";
    
    // Запрос к MongoDB
    using userver::formats::bson::MakeDoc;
    auto users_collection = pool_->GetCollection("users");
    
    auto cursor = users_collection.Find(
        MakeDoc("first_name", MakeDoc("$regex", ".*" + first_name + ".*", "$options", "i"),
                "last_name", MakeDoc("$regex", ".*" + last_name + ".*", "$options", "i"))
    );
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& doc : cursor) {
        userver::formats::json::ValueBuilder user_json;
        user_json["id"] = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
        user_json["login"] = doc["login"].template As<std::string>();
        user_json["first_name"] = doc["first_name"].template As<std::string>();
        user_json["last_name"] = doc["last_name"].template As<std::string>();
        user_json["email"] = doc["email"].template As<std::string>();
        user_json["created_at"] = std::to_string(
            doc["created_at"].template As<userver::formats::bson::Oid>().GetTimePoint().time_since_epoch().count()
        );
        response.PushBack(user_json.ExtractValue());
    }
    
    std::string json_response = userver::formats::json::ToString(response.ExtractValue());
    
    // Сохраняем в кеш на 300 секунд
    redis.Set(cache_key, json_response, 300);
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return json_response;
}

} // namespace handlers