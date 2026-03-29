#include "auth_middleware.hpp"
#include <userver/server/http/http_request.hpp>
#include <memory>

namespace auth {

std::optional<std::pair<std::string, std::string>> AuthMiddleware::Authenticate(
    const userver::server::http::HttpRequest& request) {
    
    const auto& auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        return std::nullopt;
    }
    
    std::string token = auth_header.substr(7);
    
    static JWTManager jwt_manager("your-secret-key");
    return jwt_manager.ValidateToken(token);
}

} // namespace auth