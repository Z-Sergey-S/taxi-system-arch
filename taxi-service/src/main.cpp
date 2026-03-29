#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "storage/storage_component.hpp"
#include "handlers/register_user.hpp"
#include "handlers/find_user_by_login.hpp"
#include "handlers/search_users_by_name.hpp"
#include "handlers/register_driver.hpp"
#include "handlers/create_ride.hpp"
#include "handlers/get_active_rides.hpp"
#include "handlers/accept_ride.hpp"
#include "handlers/get_user_rides.hpp"
#include "handlers/complete_ride.hpp"
#include "handlers/auth_login.hpp"
#include "handlers/auth_register.hpp"
#include "handlers/openapi_handler.hpp"
#include "handlers/swagger_ui_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>()
            .Append<storage::StorageComponent>()
            .Append<handlers::OpenApiHandler>()
            .Append<handlers::SwaggerUiHandler>()
            .Append<handlers::RegisterUserHandler>()
            .Append<handlers::FindUserByLoginHandler>()
            .Append<handlers::SearchUsersByNameHandler>()
            .Append<handlers::RegisterDriverHandler>()
            .Append<handlers::CreateRideHandler>()
            .Append<handlers::GetActiveRidesHandler>()
            .Append<handlers::AcceptRideHandler>()
            .Append<handlers::GetUserRidesHandler>()
            .Append<handlers::CompleteRideHandler>()
            .Append<handlers::AuthLoginHandler>()
            .Append<handlers::AuthRegisterHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}