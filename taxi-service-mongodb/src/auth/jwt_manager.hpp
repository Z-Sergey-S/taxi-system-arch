#pragma once
#include <string>
#include <optional>
#include <utility>

namespace auth {

class JWTManager {
public:
    explicit JWTManager(const std::string& secret);
    
    std::string GenerateToken(const std::string& user_id, const std::string& login);
    std::optional<std::pair<std::string, std::string>> ValidateToken(const std::string& token);
    
private:
    std::string secret_;
    
    std::string EncodeBase64(const std::string& input);
    std::string DecodeBase64(const std::string& input);
};

} // namespace auth