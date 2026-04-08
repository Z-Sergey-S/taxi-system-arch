#include "accept_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/ride_repository.hpp"
#include "../auth/auth_middleware.hpp"

namespace handlers {

AcceptRideHandler::AcceptRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string AcceptRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& ride_id = request.GetPathArg("ride_id");
    
    // Аутентификация (водитель)
    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver not authenticated";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    const auto& driver_id = auth_result->first;
    
    db::RideRepository repository(pg_cluster_);
    bool success = repository.AcceptRide(ride_id, driver_id);
    
    if (!success) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Cannot accept ride: ride not found, already accepted, or driver not available";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    userver::formats::json::ValueBuilder response;
    response["message"] = "Ride accepted successfully";
    response["ride_id"] = ride_id;
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers