#include "register_user.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../models/user.hpp"

namespace handlers {

RegisterUserHandler::RegisterUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string RegisterUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseCreateRequest(request_body);
        
        auto user = storage_.CreateUser(create_request);
        
        models::User::Response response;
        response.id = user.id;
        response.login = user.login;
        response.first_name = user.first_name;
        response.last_name = user.last_name;
        response.email = user.email;
        response.created_at = user.created_at;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(Serialize(response));
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers