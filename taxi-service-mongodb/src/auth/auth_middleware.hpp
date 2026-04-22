#pragma once
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_request.hpp>
#include "jwt_manager.hpp"

namespace auth {

class AuthMiddleware {
public:
    static std::optional<std::pair<std::string, std::string>> Authenticate(
        const userver::server::http::HttpRequest& request);
};

} // namespace auth