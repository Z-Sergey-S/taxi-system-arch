#include "find_user_by_login.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/user_repository.hpp"
#include "../models/user.hpp"

namespace handlers {

FindUserByLoginHandler::FindUserByLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string FindUserByLoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& login = request.GetPathArg("login");
    
    db::UserRepository repository(pg_cluster_);
    auto user = repository.FindUserByLogin(login);
    
    if (!user.has_value()) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder error;
        error["error"] = "User with login '" + login + "' not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    models::User::Response response;
    response.id = user->id;
    response.login = user->login;
    response.first_name = user->first_name;
    response.last_name = user->last_name;
    response.email = user->email;
    response.created_at = user->created_at;
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(models::Serialize(response));
}

} // namespace handlers