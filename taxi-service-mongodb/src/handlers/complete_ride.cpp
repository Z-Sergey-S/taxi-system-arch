#include "complete_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../auth/auth_middleware.hpp"

namespace handlers {

CompleteRideHandler::CompleteRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-taxi").GetPool()) {}

std::string CompleteRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& ride_id = request.GetPathArg("ride_id");
    
    // Аутентификация
    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver not authenticated";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    const auto& driver_id = auth_result->first;
    
    using userver::formats::bson::MakeDoc;
    auto rides_collection = pool_->GetCollection("rides");
    auto drivers_collection = pool_->GetCollection("drivers");
    
    // Проверяем существование поездки и её статус
    auto ride_doc = rides_collection.FindOne(MakeDoc("_id", userver::formats::bson::Oid(ride_id)));
    if (!ride_doc) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Ride not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    auto ride = *ride_doc;
    if (ride["status"].template As<std::string>() != "accepted") {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Ride is not in accepted state";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    // Проверяем, что водитель является исполнителем поездки
    if (ride["driver_id"].template As<userver::formats::bson::Oid>().ToString() != driver_id) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
        userver::formats::json::ValueBuilder error;
        error["error"] = "You are not the driver of this ride";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    // Завершаем поездку
    rides_collection.UpdateOne(
        MakeDoc("_id", userver::formats::bson::Oid(ride_id)),
        MakeDoc("$set", MakeDoc("status", "completed",
                                "completed_at", userver::formats::bson::Oid()))
    );
    
    // Освобождаем водителя
    drivers_collection.UpdateOne(
        MakeDoc("_id", userver::formats::bson::Oid(driver_id)),
        MakeDoc("$set", MakeDoc("status", "free"))
    );
    
    userver::formats::json::ValueBuilder response;
    response["message"] = "Ride completed successfully";
    response["ride_id"] = ride_id;
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers