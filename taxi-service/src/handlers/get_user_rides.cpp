#include "get_user_rides.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/ride_repository.hpp"
#include "../models/ride.hpp"

namespace handlers {

GetUserRidesHandler::GetUserRidesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string GetUserRidesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& user_id = request.GetPathArg("user_id");
    
    db::RideRepository repository(pg_cluster_);
    auto rides = repository.GetUserRideHistory(user_id);
    
    userver::formats::json::ValueBuilder response(userver::formats::json::Type::kArray);
    
    for (const auto& ride : rides) {
        userver::formats::json::ValueBuilder ride_json;
        ride_json["id"] = ride.id;
        ride_json["user_id"] = ride.user_id;
        if (ride.driver_id) {
            ride_json["driver_id"] = *ride.driver_id;
        }
        ride_json["start_address"] = ride.start_address;
        ride_json["end_address"] = ride.end_address;
        ride_json["status"] = models::RideStatusToString(ride.status);
        ride_json["price"] = ride.price;
        ride_json["created_at"] = ride.created_at;
        if (ride.accepted_at) {
            ride_json["accepted_at"] = *ride.accepted_at;
        }
        if (ride.completed_at) {
            ride_json["completed_at"] = *ride.completed_at;
        }
        response.PushBack(ride_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers