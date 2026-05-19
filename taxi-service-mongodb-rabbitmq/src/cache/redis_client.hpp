#pragma once

#include <string>
#include <memory>
#include <optional>
#include <hiredis/hiredis.h>

namespace cache {

class RedisClient {
public:
    static RedisClient& GetInstance();
    
    bool Connect(const std::string& host, int port);
    void Disconnect();
    bool IsConnected() const;

    bool Set(const std::string& key, const std::string& value, int ttl_seconds = 0);
    std::optional<std::string> Get(const std::string& key);
    bool Del(const std::string& key);
    bool Exists(const std::string& key);
    bool Expire(const std::string& key, int seconds);

private:
    RedisClient();
    ~RedisClient();
    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;

    redisContext* context_;
    bool connected_;
};

} // namespace cache