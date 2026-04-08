#include "register_driver.hpp"
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/formats/json/serialize.hpp>
#include "../db/driver_repository.hpp"
#include "../models/driver.hpp"

namespace handlers {

RegisterDriverHandler::RegisterDriverHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()) {}

std::string RegisterDriverHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    try {
        const auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto create_request = models::ParseDriverCreateRequest(request_body);
        
        db::DriverRepository repository(pg_cluster_);
        auto driver = repository.CreateDriver(create_request);
        
        models::Driver::Response response;
        response.id = driver.id;
        response.first_name = driver.first_name;
        response.last_name = driver.last_name;
        response.car_model = driver.car_model;
        response.car_number = driver.car_number;
        response.status = models::DriverStatusToString(driver.status);
        response.rating = driver.rating;
        response.created_at = driver.created_at;
        
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