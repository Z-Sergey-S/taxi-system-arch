#include "get_active_rides.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/auth_middleware.hpp"

namespace handlers {

GetActiveRidesHandler::GetActiveRidesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string GetActiveRidesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    // Аутентификация
    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Unauthorized";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    using userver::formats::bson::MakeDoc;
    namespace options = userver::storages::mongo::options;
    
    auto rides_collection = pool_->GetCollection("rides");
    
    // Используем два отдельных запроса вместо $or
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    // Сначала ищем created
    auto cursor_created = rides_collection.Find(
        MakeDoc("status", "created"),
        options::Sort{std::make_pair("created_at", options::Sort::kAscending)}
    );
    
    for (const auto& doc : cursor_created) {
        userver::formats::json::ValueBuilder ride_json;
        ride_json["id"] = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["user_id"] = doc["user_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["start_address"] = doc["start_address"].template As<std::string>();
        ride_json["end_address"] = doc["end_address"].template As<std::string>();
        ride_json["status"] = doc["status"].template As<std::string>();
        ride_json["price"] = doc["price"].template As<double>();
        ride_json["created_at"] = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        response.PushBack(ride_json.ExtractValue());
    }
    
    // Затем ищем accepted
    auto cursor_accepted = rides_collection.Find(
        MakeDoc("status", "accepted"),
        options::Sort{std::make_pair("created_at", options::Sort::kAscending)}
    );
    
    for (const auto& doc : cursor_accepted) {
        userver::formats::json::ValueBuilder ride_json;
        ride_json["id"] = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["user_id"] = doc["user_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["start_address"] = doc["start_address"].template As<std::string>();
        ride_json["end_address"] = doc["end_address"].template As<std::string>();
        ride_json["status"] = doc["status"].template As<std::string>();
        ride_json["price"] = doc["price"].template As<double>();
        ride_json["created_at"] = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        response.PushBack(ride_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers