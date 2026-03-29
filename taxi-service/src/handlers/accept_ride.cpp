#include "accept_ride.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../auth/auth_middleware.hpp"

namespace handlers {

AcceptRideHandler::AcceptRideHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string AcceptRideHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    const auto& ride_id = request.GetPathArg("ride_id");
    
    // Authenticate driver
    auto auth_result = auth::AuthMiddleware::Authenticate(request);
    if (!auth_result) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder error;
        error["error"] = "Driver not authenticated";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    const auto& driver_id = auth_result->first;
    
    bool success = storage_.AcceptRide(ride_id, driver_id);
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