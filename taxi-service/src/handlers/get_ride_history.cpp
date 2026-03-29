#include "get_ride_history.hpp"
#include <userver/formats/json/serialize.hpp>

namespace handlers {

GetRideHistoryHandler::GetRideHistoryHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string GetRideHistoryHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    userver::formats::json::ValueBuilder response;
    response["message"] = "Get ride history endpoint";
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers
