#pragma once

#include <string>
#include <userver/server/http/http_request.hpp>

namespace rate_limit {

class RateLimitMiddleware {
public:
    static bool CheckLimit(const std::string& user_key,
                           int& remaining,
                           int& limit,
                           int& reset);
    
    static std::string GetUserKey(const userver::server::http::HttpRequest& request);
};

} // namespace rate_limit