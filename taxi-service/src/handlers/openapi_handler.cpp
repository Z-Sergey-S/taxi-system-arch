#include "openapi_handler.hpp"
#include <fstream>
#include <sstream>
#include <userver/formats/json/serialize.hpp>

namespace handlers {

OpenApiHandler::OpenApiHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string OpenApiHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    std::ifstream file("openapi.yaml");
    if (!file.is_open()) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder error;
        error["error"] = "openapi.yaml not found";
        return userver::formats::json::ToString(error.ExtractValue());
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return buffer.str();
}

} // namespace handlers