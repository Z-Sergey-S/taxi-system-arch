#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/logging/log.hpp>
#include <cstdlib>

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
#include "cache/redis_client.hpp"
#include "rate_limit/token_bucket.hpp"

int main(int argc, char* argv[]) {
    // Инициализация Redis (с логами)
    const char* redis_host = std::getenv("REDIS_HOST");
    if (!redis_host) {
        redis_host = "redis";  // Используем имя сервиса из docker-compose
    }
    
    const char* redis_port_str = std::getenv("REDIS_PORT");
    int redis_port = redis_port_str ? std::atoi(redis_port_str) : 6379;
    
    LOG_INFO() << "Connecting to Redis at " << redis_host << ":" << redis_port;
    
    auto& redis_client = cache::RedisClient::GetInstance();
    if (!redis_client.Connect(redis_host, redis_port)) {
        LOG_WARNING() << "Failed to connect to Redis, continuing without cache";
    } else {
        LOG_INFO() << "Redis connected successfully";
    }
    
    // Настройка rate limiting по умолчанию
    rate_limit::RateLimiter::GetInstance().SetLimit("default", 10.0 / 60.0, 10);
    LOG_INFO() << "Rate limiter initialized with default limit: 10 requests per minute";
    
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>()
            .Append<userver::components::Mongo>("mongo-taxi")
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