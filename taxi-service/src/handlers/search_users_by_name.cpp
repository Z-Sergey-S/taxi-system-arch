#include "search_users_by_name.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../models/user.hpp"

namespace handlers {

SearchUsersByNameHandler::SearchUsersByNameHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string SearchUsersByNameHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& first_name = request.GetArg("first_name");
    const auto& last_name = request.GetArg("last_name");
    
    std::string mask = first_name + " " + last_name;
    auto users = storage_.FindUsersByNameMask(mask);
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& user : users) {
        userver::formats::json::ValueBuilder user_json;
        user_json["id"] = user.id;
        user_json["login"] = user.login;
        user_json["first_name"] = user.first_name;
        user_json["last_name"] = user.last_name;
        user_json["email"] = user.email;
        response.PushBack(user_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers