#include "register_driver.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../models/driver.hpp"

namespace handlers {

RegisterDriverHandler::RegisterDriverHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string RegisterDriverHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseDriverCreateRequest(request_body);
        
        using userver::formats::bson::MakeDoc;
        auto drivers_collection = pool_->GetCollection("drivers");
        
        // Проверка на существование водителя по логину
        auto existing_driver = drivers_collection.FindOne(MakeDoc("login", create_request.login));
        if (existing_driver) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder error;
            error["error"] = "Driver with this login already exists";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        // Хешируем пароль
        std::string password_hash = "hash_" + create_request.password;
        
        // Вставка водителя
        drivers_collection.InsertOne(
            MakeDoc("login", create_request.login,
                    "first_name", create_request.first_name,
                    "last_name", create_request.last_name,
                    "car_model", create_request.car_model,
                    "car_number", create_request.car_number,
                    "password_hash", password_hash,
                    "status", "free",
                    "rating", 5.0,
                    "created_at", userver::formats::bson::Oid())
        );
        
        // Находим только что созданного водителя
        auto new_driver = drivers_collection.FindOne(MakeDoc("login", create_request.login));
        std::string driver_id;
        if (new_driver) {
            driver_id = new_driver->operator[]("_id").template As<userver::formats::bson::Oid>().ToString();
        }
        
        models::Driver::Response response;
        response.id = driver_id;
        response.first_name = create_request.first_name;
        response.last_name = create_request.last_name;
        response.car_model = create_request.car_model;
        response.car_number = create_request.car_number;
        response.status = "free";
        response.rating = 5.0;
        response.created_at = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
        );
        
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