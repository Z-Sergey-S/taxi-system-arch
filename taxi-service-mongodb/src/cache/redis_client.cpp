#include "redis_client.hpp"
#include <iostream>
#include <cstring>

namespace cache {

RedisClient::RedisClient() : context_(nullptr), connected_(false) {}

RedisClient::~RedisClient() {
    Disconnect();
}

RedisClient& RedisClient::GetInstance() {
    static RedisClient instance;
    return instance;
}

bool RedisClient::Connect(const std::string& host, int port) {
    if (connected_) {
        Disconnect();
    }

    struct timeval timeout = { 1, 500000 };
    context_ = redisConnectWithTimeout(host.c_str(), port, timeout);

    if (context_ == nullptr || context_->err) {
        if (context_) {
            std::cerr << "Redis connection error: " << context_->errstr << std::endl;
            redisFree(context_);
            context_ = nullptr;
        } else {
            std::cerr << "Redis connection error: can't allocate redis context" << std::endl;
        }
        connected_ = false;
        return false;
    }

    connected_ = true;
    std::cout << "Redis connected to " << host << ":" << port << std::endl;
    return true;
}

void RedisClient::Disconnect() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
    connected_ = false;
}

bool RedisClient::IsConnected() const {
    return connected_ && context_ != nullptr;
}

bool RedisClient::Set(const std::string& key, const std::string& value, int ttl_seconds) {
    if (!IsConnected()) {
        return false;
    }

    redisReply* reply = nullptr;

    if (ttl_seconds > 0) {
        reply = (redisReply*)redisCommand(context_,
            "SETEX %s %d %s", key.c_str(), ttl_seconds, value.c_str());
    } else {
        reply = (redisReply*)redisCommand(context_,
            "SET %s %s", key.c_str(), value.c_str());
    }

    if (reply == nullptr) {
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_STATUS && 
                    strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);
    return success;
}

std::optional<std::string> RedisClient::Get(const std::string& key) {
    if (!IsConnected()) {
        return std::nullopt;
    }

    redisReply* reply = (redisReply*)redisCommand(context_,
        "GET %s", key.c_str());

    if (reply == nullptr) {
        return std::nullopt;
    }

    std::optional<std::string> result;
    if (reply->type == REDIS_REPLY_STRING) {
        result = std::string(reply->str, reply->len);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisClient::Del(const std::string& key) {
    if (!IsConnected()) {
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(context_,
        "DEL %s", key.c_str());

    if (reply == nullptr) {
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    freeReplyObject(reply);
    return success;
}

bool RedisClient::Exists(const std::string& key) {
    if (!IsConnected()) {
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(context_,
        "EXISTS %s", key.c_str());

    if (reply == nullptr) {
        return false;
    }

    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    freeReplyObject(reply);
    return exists;
}

bool RedisClient::Expire(const std::string& key, int seconds) {
    if (!IsConnected()) {
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(context_,
        "EXPIRE %s %d", key.c_str(), seconds);

    if (reply == nullptr) {
        return false;
    }

    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
    freeReplyObject(reply);
    return success;
}

} // namespace cache