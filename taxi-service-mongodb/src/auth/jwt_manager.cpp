#include "jwt_manager.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace auth {

JWTManager::JWTManager(const std::string& secret) : secret_(secret) {}

std::string JWTManager::EncodeBase64(const std::string& input) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string encoded;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    for (char c : input) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (int j = 0; j < 4; j++)
                encoded += base64_chars[char_array_4[j]];
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            encoded += base64_chars[char_array_4[j]];
        
        while (i++ < 3)
            encoded += '=';
    }
    
    return encoded;
}

std::string JWTManager::DecodeBase64(const std::string& input) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string decoded;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    // Removed unused variable 'in_len'
    
    for (char c : input) {
        if (c == '=') break;
        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) continue;
        char_array_4[i++] = pos;
        if (i == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (int j = 0; j < 3; j++)
                decoded += char_array_3[j];
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 4; j++)
            char_array_4[j] = 0;
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        for (int j = 0; j < i - 1; j++)
            decoded += char_array_3[j];
    }
    
    return decoded;
}

std::string JWTManager::GenerateToken(const std::string& user_id, const std::string& login) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(24);
    
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto exp_time = std::chrono::system_clock::to_time_t(exp);
    
    std::stringstream header;
    header << R"({"alg":"HS256","typ":"JWT"})";
    
    std::stringstream payload;
    payload << R"({"user_id":")" << user_id << R"(","login":")" << login 
            << R"(","iat":)" << now_time << R"(,"exp":)" << exp_time << "}";
    
    std::string encoded_header = EncodeBase64(header.str());
    std::string encoded_payload = EncodeBase64(payload.str());
    
    std::string signature = EncodeBase64(secret_ + user_id + login);
    
    return encoded_header + "." + encoded_payload + "." + signature;
}

std::optional<std::pair<std::string, std::string>> JWTManager::ValidateToken(const std::string& token) {
    size_t first_dot = token.find('.');
    size_t last_dot = token.rfind('.');
    
    if (first_dot == std::string::npos || last_dot == std::string::npos || first_dot == last_dot) {
        return std::nullopt;
    }
    
    std::string encoded_payload = token.substr(first_dot + 1, last_dot - first_dot - 1);
    std::string signature = token.substr(last_dot + 1);
    
    std::string payload = DecodeBase64(encoded_payload);
    
    size_t user_id_pos = payload.find("\"user_id\":\"");
    if (user_id_pos == std::string::npos) return std::nullopt;
    user_id_pos += 11;
    size_t user_id_end = payload.find('"', user_id_pos);
    if (user_id_end == std::string::npos) return std::nullopt;
    std::string user_id = payload.substr(user_id_pos, user_id_end - user_id_pos);
    
    size_t login_pos = payload.find("\"login\":\"");
    if (login_pos == std::string::npos) return std::nullopt;
    login_pos += 9;
    size_t login_end = payload.find('"', login_pos);
    if (login_end == std::string::npos) return std::nullopt;
    std::string login = payload.substr(login_pos, login_end - login_pos);
    
    size_t exp_pos = payload.find("\"exp\":");
    if (exp_pos != std::string::npos) {
        exp_pos += 6;
        size_t exp_end = payload.find(',', exp_pos);
        if (exp_end == std::string::npos) exp_end = payload.find('}', exp_pos);
        if (exp_end != std::string::npos) {
            long exp_time = std::stol(payload.substr(exp_pos, exp_end - exp_pos));
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            if (now > exp_time) {
                return std::nullopt; 
            }
        }
    }
    
    std::string expected_signature = EncodeBase64(secret_ + user_id + login);
    if (signature != expected_signature) {
        return std::nullopt;
    }
    
    return std::make_pair(user_id, login);
}

} // namespace auth