#include "auth_register.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../models/user.hpp"
#include "../auth/jwt_manager.hpp"

namespace handlers {

AuthRegisterHandler::AuthRegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string AuthRegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseCreateRequest(request_body);
        
        auto user = storage_.CreateUser(create_request);
        
        static auth::JWTManager jwt_manager("your-secret-key");
        std::string token = jwt_manager.GenerateToken(user.id, user.login);
        
        userver::formats::json::ValueBuilder response;
        response["token"] = token;
        response["user_id"] = user.id;
        response["login"] = user.login;
        response["first_name"] = user.first_name;
        response["last_name"] = user.last_name;
        response["email"] = user.email;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(response.ExtractValue());
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers