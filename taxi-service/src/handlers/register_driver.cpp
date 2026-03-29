#include "register_driver.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../storage/storage_component.hpp"
#include "../models/driver.hpp"

namespace handlers {

RegisterDriverHandler::RegisterDriverHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<storage::StorageComponent>().GetStorage()) {}

std::string RegisterDriverHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseDriverCreateRequest(request_body);
        
        auto driver = storage_.CreateDriver(create_request);
        
        models::Driver::Response response;
        response.id = driver.id;
        response.first_name = driver.first_name;
        response.last_name = driver.last_name;
        response.car_model = driver.car_model;
        response.car_number = driver.car_number;
        response.status = DriverStatusToString(driver.status);
        response.rating = driver.rating;
        response.created_at = driver.created_at;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(Serialize(response));
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder error;
        error["error"] = e.what();
        return userver::formats::json::ToString(error.ExtractValue());
    }
}

} // namespace handlers