#include "auth_login.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/jwt_manager.hpp"

namespace handlers {

AuthLoginHandler::AuthLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string AuthLoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        std::string login = request_body["login"].As<std::string>();
        std::string password = request_body["password"].As<std::string>();
        
        using userver::formats::bson::MakeDoc;
        
        std::string password_hash = "hash_" + password;
        std::string user_id;
        std::string user_login = login;
        
        // Сначала ищем в коллекции users
        auto users_collection = pool_->GetCollection("users");
        auto user_doc = users_collection.FindOne(MakeDoc("login", login, "password_hash", password_hash));
        
        if (user_doc) {
            // Пользователь найден в коллекции users
            auto doc = *user_doc;
            user_id = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
            user_login = doc["login"].template As<std::string>();
        } else {
            // Если не нашли в users, ищем в коллекции drivers
            auto drivers_collection = pool_->GetCollection("drivers");
            auto driver_doc = drivers_collection.FindOne(MakeDoc("login", login, "password_hash", password_hash));
            
            if (driver_doc) {
                auto doc = *driver_doc;
                user_id = doc["_id"].template As<userver::formats::bson::Oid>().ToString();
                user_login = doc["login"].template As<std::string>();
            } else {
                request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
                userver::formats::json::ValueBuilder error;
                error["error"] = "Invalid credentials";
                return userver::formats::json::ToString(error.ExtractValue());
            }
        }
        
        static auth::JWTManager jwt_manager("your-secret-key");
        std::string token = jwt_manager.GenerateToken(user_id, user_login);
        
        userver::formats::json::ValueBuilder response;
        response["token"] = token;
        response["user_id"] = user_id;
        response["login"] = user_login;
        
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