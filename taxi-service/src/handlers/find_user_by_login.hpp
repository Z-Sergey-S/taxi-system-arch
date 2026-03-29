#pragma once
#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include "../storage/taxi_storage.hpp"

namespace handlers {

class FindUserByLoginHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-find-user-by-login";

    FindUserByLoginHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context);

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override;

private:
    storage::TaxiStorage& storage_;
};

} // namespace handlers