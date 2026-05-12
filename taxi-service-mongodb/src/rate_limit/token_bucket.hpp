#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace rate_limit {

class TokenBucket {
public:
    TokenBucket(double rate_per_second, int capacity);
    
    bool TryConsume(int tokens = 1);
    void Refill();
    double GetCurrentTokens() const;
    int GetCapacity() const;
    double GetRate() const;
    std::chrono::steady_clock::time_point GetLastRefill() const;
    
private:
    double rate_per_second_;
    int capacity_;
    double tokens_;
    std::chrono::steady_clock::time_point last_refill_;
    mutable std::mutex mutex_;
};

class RateLimiter {
public:
    static RateLimiter& GetInstance();
    
    bool IsAllowed(const std::string& key, int tokens = 1);
    int GetRemainingTokens(const std::string& key);
    int GetCapacity(const std::string& key);
    int GetResetTimeSeconds(const std::string& key);
    
    void SetLimit(const std::string& key, double rate_per_second, int capacity);
    
private:
    RateLimiter() = default;
    std::unordered_map<std::string, std::shared_ptr<TokenBucket>> buckets_;
    std::mutex mutex_;
};

} // namespace rate_limit