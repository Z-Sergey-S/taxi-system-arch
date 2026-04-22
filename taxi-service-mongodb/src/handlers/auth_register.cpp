#include "auth_register.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/user.hpp"
#include "../auth/jwt_manager.hpp"

namespace handlers {

AuthRegisterHandler::AuthRegisterHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string AuthRegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseCreateRequest(request_body);
        
        using userver::formats::bson::MakeDoc;
        auto users_collection = pool_->GetCollection("users");
        
        // Проверка на существование пользователя
        auto existing_user = users_collection.FindOne(MakeDoc("login", create_request.login));
        if (existing_user) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder error;
            error["error"] = "User with this login already exists";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        std::string password_hash = "hash_" + create_request.password;
        
        // Вставка пользователя
        auto result = users_collection.InsertOne(
            MakeDoc("login", create_request.login,
                    "first_name", create_request.first_name,
                    "last_name", create_request.last_name,
                    "email", create_request.email,
                    "password_hash", password_hash)
        );
        
        auto upserted_ids = result.UpsertedIds();
        std::string user_id;
        if (!upserted_ids.empty()) {
            user_id = upserted_ids.begin()->second.template As<userver::formats::bson::Oid>().ToString();
        }
        
        static auth::JWTManager jwt_manager("your-secret-key");
        std::string token = jwt_manager.GenerateToken(user_id, create_request.login);
        
        userver::formats::json::ValueBuilder response;
        response["token"] = token;
        response["user_id"] = user_id;
        response["login"] = create_request.login;
        response["first_name"] = create_request.first_name;
        response["last_name"] = create_request.last_name;
        response["email"] = create_request.email;
        
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