#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace events {
class EventProducer;
}

namespace handlers {

class CreateRideHandler : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-ride";

    CreateRideHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override;

private:
    userver::storages::mongo::PoolPtr pool_;
    events::EventProducer* event_producer_;
};

} // namespace handlers