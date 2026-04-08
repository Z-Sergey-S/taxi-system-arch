#include "search_users_by_name.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/user_repository.hpp"
#include "../models/user.hpp"

namespace handlers {

SearchUsersByNameHandler::SearchUsersByNameHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string SearchUsersByNameHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& first_name = request.GetArg("first_name");
    const auto& last_name = request.GetArg("last_name");
    
    db::UserRepository repository(pg_cluster_);
    auto users = repository.FindUsersByNameMask(first_name, last_name);
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& user : users) {
        userver::formats::json::ValueBuilder user_json;
        user_json["id"] = user.id;
        user_json["login"] = user.login;
        user_json["first_name"] = user.first_name;
        user_json["last_name"] = user.last_name;
        user_json["email"] = user.email;
        user_json["created_at"] = user.created_at;
        response.PushBack(user_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers