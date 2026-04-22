#include "get_user_rides.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>

namespace handlers {

GetUserRidesHandler::GetUserRidesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string GetUserRidesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& user_id = request.GetPathArg("user_id");
    
    using userver::formats::bson::MakeDoc;
    namespace options = userver::storages::mongo::options;
    
    auto rides_collection = pool_->GetCollection("rides");
    
    auto cursor = rides_collection.Find(
        MakeDoc("user_id", userver::formats::bson::Oid(user_id)),
        options::Sort{std::make_pair("created_at", options::Sort::kDescending)}
    );
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& doc : cursor) {
        userver::formats::json::ValueBuilder ride_json;
        ride_json["id"] = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
        ride_json["user_id"] = doc["user_id"].template As<userver::formats::bson::Oid>().ToString();
        
        auto driver_id_val = doc["driver_id"];
        if (!driver_id_val.IsNull()) {
            ride_json["driver_id"] = driver_id_val.template As<userver::formats::bson::Oid>().ToString();
        }
        
        ride_json["start_address"] = doc["start_address"].template As<std::string>();
        ride_json["end_address"] = doc["end_address"].template As<std::string>();
        ride_json["status"] = doc["status"].template As<std::string>();
        ride_json["price"] = doc["price"].template As<double>();
        ride_json["created_at"] = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        
        auto accepted_at_val = doc["accepted_at"];
        if (!accepted_at_val.IsNull()) {
            ride_json["accepted_at"] = std::to_string(
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
            );
        }
        
        auto completed_at_val = doc["completed_at"];
        if (!completed_at_val.IsNull()) {
            ride_json["completed_at"] = std::to_string(
                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
            );
        }
        
        response.PushBack(ride_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers