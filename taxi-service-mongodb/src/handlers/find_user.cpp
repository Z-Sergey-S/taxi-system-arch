#include "find_user.hpp"
#include <userver/formats/json/serialize.hpp>

namespace handlers {

FindUserHandler::FindUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string FindUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    userver::formats::json::ValueBuilder response;
    response["message"] = "Find user endpoint";
    return userver::formats::json::ToString(response.ExtractValue());
}

} // namespace handlers
