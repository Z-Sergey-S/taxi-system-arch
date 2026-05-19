#include "register_user.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/user.hpp"
#include "../events/event_producer.hpp"

namespace handlers {

RegisterUserHandler::RegisterUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()),
      event_producer_(&context.FindComponent<events::EventProducer>()) {}

std::string RegisterUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseCreateRequest(request_body);
        
        using userver::formats::bson::MakeDoc;
        auto users_collection = pool_->GetCollection("users");
        
        auto existing_user = users_collection.FindOne(MakeDoc("login", create_request.login));
        if (existing_user) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder error;
            error["error"] = "User with this login already exists";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        std::string password_hash = "hash_" + create_request.password;
        
        users_collection.InsertOne(
            MakeDoc("login", create_request.login,
                    "first_name", create_request.first_name,
                    "last_name", create_request.last_name,
                    "email", create_request.email,
                    "password_hash", password_hash,
                    "created_at", userver::formats::bson::Oid())
        );
        
        auto new_user = users_collection.FindOne(MakeDoc("login", create_request.login));
        std::string user_id;
        if (new_user) {
            user_id = new_user->operator[]("_id").template As<userver::formats::bson::Oid>().ToString();
        }
        
        models::User::Response response;
        response.id = user_id;
        response.login = create_request.login;
        response.first_name = create_request.first_name;
        response.last_name = create_request.last_name;
        response.email = create_request.email;
        response.created_at = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        
        event_producer_->PublishUserCreated(user_id, create_request.login,
                                            create_request.first_name,
                                            create_request.last_name,
                                            create_request.email);
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(models::Serialize(response));
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers