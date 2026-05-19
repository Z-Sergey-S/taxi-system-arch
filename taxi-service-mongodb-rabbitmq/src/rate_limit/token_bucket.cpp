#include "token_bucket.hpp"
#include <algorithm>
#include <userver/logging/log.hpp>

namespace rate_limit {

TokenBucket::TokenBucket(double rate_per_second, int capacity)
    : rate_per_second_(rate_per_second),
      capacity_(capacity),
      tokens_(static_cast<double>(capacity)),
      last_refill_(std::chrono::steady_clock::now()) {}

bool TokenBucket::TryConsume(int tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    Refill();
    
    LOG_INFO() << "TokenBucket::TryConsume: tokens_=" << tokens_ 
               << " capacity=" << capacity_ 
               << " need=" << tokens
               << " rate=" << rate_per_second_;
    
    if (tokens_ >= static_cast<double>(tokens)) {
        tokens_ -= static_cast<double>(tokens);
        LOG_INFO() << "TokenBucket::TryConsume: CONSUMED, remaining=" << tokens_;
        return true;
    }
    LOG_INFO() << "TokenBucket::TryConsume: DENIED, available=" << tokens_;
    return false;
}

void TokenBucket::Refill() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - last_refill_).count();
    
    if (elapsed > 0) {
        double new_tokens = elapsed * rate_per_second_;
        tokens_ = (std::min)(static_cast<double>(capacity_), tokens_ + new_tokens);
        last_refill_ = now;
    }
}

double TokenBucket::GetCurrentTokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

int TokenBucket::GetCapacity() const {
    return capacity_;
}

double TokenBucket::GetRate() const {
    return rate_per_second_;
}

std::chrono::steady_clock::time_point TokenBucket::GetLastRefill() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_refill_;
}

RateLimiter& RateLimiter::GetInstance() {
    static RateLimiter instance;
    return instance;
}

bool RateLimiter::IsAllowed(const std::string& key, int tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = buckets_.find(key);
    if (it == buckets_.end()) {
        LOG_INFO() << "RateLimiter::IsAllowed: bucket for key '" << key << "' not found, creating with default limit";
        // Создаем бакет с дефолтными значениями: 10 запросов в минуту = 10/60 в секунду
        double rate_per_second = 10.0 / 60.0;  // 10 запросов в минуту
        int capacity = 10;
        buckets_[key] = std::make_shared<TokenBucket>(rate_per_second, capacity);
        it = buckets_.find(key);
    }
    
    bool result = it->second->TryConsume(tokens);
    LOG_INFO() << "RateLimiter::IsAllowed: key=" << key << " result=" << result;
    return result;
}

int RateLimiter::GetRemainingTokens(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = buckets_.find(key);
    if (it == buckets_.end()) {
        return 10;  // Возвращаем максимальное количество, если бакет не создан
    }
    
    return static_cast<int>(it->second->GetCurrentTokens());
}

int RateLimiter::GetCapacity(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = buckets_.find(key);
    if (it == buckets_.end()) {
        return 10;
    }
    
    return it->second->GetCapacity();
}

int RateLimiter::GetResetTimeSeconds(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = buckets_.find(key);
    if (it == buckets_.end()) {
        return 60;
    }
    
    double current_tokens = it->second->GetCurrentTokens();
    double rate = it->second->GetRate();
    int capacity = it->second->GetCapacity();
    
    // Время до полного восстановления = (capacity - current_tokens) / rate
    if (rate > 0 && current_tokens < capacity) {
        int reset = static_cast<int>((capacity - current_tokens) / rate);
        return (std::max)(1, reset);
    }
    
    return 60;
}

void RateLimiter::SetLimit(const std::string& key, double rate_per_second, int capacity) {
    std::lock_guard<std::mutex> lock(mutex_);
    LOG_INFO() << "RateLimiter::SetLimit: key=" << key << " rate=" << rate_per_second << " capacity=" << capacity;
    buckets_[key] = std::make_shared<TokenBucket>(rate_per_second, capacity);
}

} // namespace rate_limit