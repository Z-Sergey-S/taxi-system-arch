#pragma once

#include <string>

namespace cache {

inline std::string UserLoginKey(const std::string& login) {
    return "user:login:" + login;
}

inline std::string UserSearchKey(const std::string& first_name, const std::string& last_name) {
    return "user:search:" + first_name + ":" + last_name;
}

inline std::string ActiveRidesKey() {
    return "rides:active";
}

inline std::string UserRidesKey(const std::string& user_id) {
    return "user:rides:" + user_id;
}

} // namespace cache