#include "create_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/ride_repository.hpp"
#include "../db/user_repository.hpp"
#include "../auth/auth_middleware.hpp"
#include "../models/ride.hpp"

namespace handlers {

CreateRideHandler::CreateRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string CreateRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        // Аутентификация
        auto auth_result = auth::AuthMiddleware::Authenticate(request);
        if (!auth_result) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder error;
            error["error"] = "Unauthorized";
            return userver::formats::json::ToString(error.ExtractValue());
        }
        
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseRideCreateRequest(request_body);
        
        db::RideRepository repository(pg_cluster_);
        auto ride = repository.CreateRide(create_request);
        
        models::Ride::Response response;
        response.id = ride.id;
        response.user_id = ride.user_id;
        response.driver_id = ride.driver_id;
        response.start_address = ride.start_address;
        response.end_address = ride.end_address;
        response.status = models::RideStatusToString(ride.status);
        response.price = ride.price;
        response.created_at = ride.created_at;
        response.accepted_at = ride.accepted_at;
        response.completed_at = ride.completed_at;
        
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