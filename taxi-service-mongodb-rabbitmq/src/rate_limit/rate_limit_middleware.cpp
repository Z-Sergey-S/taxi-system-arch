#include "rate_limit_middleware.hpp"
#include "token_bucket.hpp"
#include <userver/logging/log.hpp>
#include <userver/server/http/http_request.hpp>

namespace rate_limit {

bool RateLimitMiddleware::CheckLimit(const std::string& user_key,
                                      int& remaining,
                                      int& limit,
                                      int& reset) {
    limit = 10;
    
    LOG_INFO() << "RateLimit Check for user: " << user_key;
    
    // Проверяем лимит - бакет создается автоматически
    bool allowed = RateLimiter::GetInstance().IsAllowed(user_key);
    remaining = RateLimiter::GetInstance().GetRemainingTokens(user_key);
    reset = RateLimiter::GetInstance().GetResetTimeSeconds(user_key);
    
    if (remaining < 0) remaining = limit;
    
    LOG_INFO() << "RateLimit result: allowed=" << allowed 
               << " remaining=" << remaining 
               << " limit=" << limit;
    
    return allowed;
}

std::string RateLimitMiddleware::GetUserKey(const userver::server::http::HttpRequest& request) {
    // Используем комбинацию заголовков для идентификации клиента
    std::string client_id = request.GetHeader("X-Client-Id");
    if (client_id.empty()) {
        client_id = request.GetHeader("User-Agent");
    }
    if (client_id.empty()) {
        client_id = "anonymous";
    }
    
    LOG_INFO() << "RateLimit user key: " << client_id;
    return client_id;
}

} // namespace rate_limit