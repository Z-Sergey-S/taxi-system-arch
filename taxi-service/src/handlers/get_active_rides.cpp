#include "get_active_rides.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../models/ride.hpp"

namespace handlers {

GetActiveRidesHandler::GetActiveRidesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string GetActiveRidesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    auto rides = storage_.GetActiveRides();
    
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
        ride_json["status"] = RideStatusToString(ride.status);
        ride_json["price"] = ride.price;
        ride_json["created_at"] = ride.created_at;
        response.PushBack(ride_json.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers