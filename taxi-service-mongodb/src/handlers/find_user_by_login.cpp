#include "find_user_by_login.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/user.hpp"

namespace handlers {

FindUserByLoginHandler::FindUserByLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string FindUserByLoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& login = request.GetPathArg("login");
    
    using userver::formats::bson::MakeDoc;
    auto users_collection = pool_->GetCollection("users");
    
    auto user_doc = users_collection.FindOne(MakeDoc("login", login));
    
    if (!user_doc) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder error;
        error["error"] = "User with login '" + login + "' not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    auto doc = *user_doc;
    
    models::User::Response response;
    response.id = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
    response.login = doc["login"].template As<std::string>();
    response.first_name = doc["first_name"].template As<std::string>();
    response.last_name = doc["last_name"].template As<std::string>();
    response.email = doc["email"].template As<std::string>();
    response.created_at = std::to_string(
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
    );
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(models::Serialize(response));
}

} // namespace handlers