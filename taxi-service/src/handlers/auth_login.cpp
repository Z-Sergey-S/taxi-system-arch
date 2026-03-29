#include "auth_login.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../auth/jwt_manager.hpp"

namespace handlers {

AuthLoginHandler::AuthLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string AuthLoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        std::string login = request_body["login"].As<std::string>();
        std::string password = request_body["password"].As<std::string>();
        
        auto user = storage_.AuthenticateUser(login, password);
        if (!user.has_value()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder error;
            error["error"] = "Invalid credentials";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        static auth::JWTManager jwt_manager("your-secret-key");
        std::string token = jwt_manager.GenerateToken(user->id, user->login);
        
        userver::formats::json::ValueBuilder response;
        response["token"] = token;
        response["user_id"] = user->id;
        response["login"] = user->login;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(response.ExtractValue());
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers