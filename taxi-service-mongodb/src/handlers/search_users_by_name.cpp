#include "search_users_by_name.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/user.hpp"

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
    
    using userver::formats::bson::MakeDoc;
    
    auto users_collection = pool_->GetCollection("users");
    
    // Поиск по регулярному выражению (регистронезависимый)
    auto cursor = users_collection.Find(
        MakeDoc("first_name", MakeDoc("$regex", ".*" + first_name + ".*", "$options", "i"),
                "last_name", MakeDoc("$regex", ".*" + last_name + ".*", "$options", "i"))
    );
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& doc : cursor) {
        userver::formats::json::ValueBuilder user_json;
        user_json["id"] = doc["_id"].As<userver::formats::bson::Oid>().ToString();
        user_json["login"] = doc["login"].As<std::string>();
        user_json["first_name"] = doc["first_name"].As<std::string>();
        user_json["last_name"] = doc["last_name"].As<std::string>();
        user_json["email"] = doc["email"].As<std::string>();
        user_json["created_at"] = std::to_string(
            doc["created_at"].As<userver::formats::bson::Oid>().GetTimePoint().time_since_epoch().count()
        );
        response.PushBack(user_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers