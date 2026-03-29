#include "taxi_storage.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <ctime>
#include <chrono>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

namespace storage {

// ============================================================================
// Private helper methods
// ============================================================================

std::string TaxiStorage::HashPassword(const std::string& password) {
    std::string salt = "taxi_service_salt_2024";
    std::string combined = password + salt;
    
    size_t hash = 0;
    for (char c : combined) {
        hash = hash * 31 + static_cast<size_t>(c);
    }
    
    return std::to_string(hash);
}

std::string TaxiStorage::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&time_t_now));
    return std::string(buffer);
}

double TaxiStorage::CalculateRidePrice(const std::string& start_address, const std::string& end_address) {
    double base_price = 50.0;
    double distance_factor = 10.0;
    double price = base_price + (start_address.length() + end_address.length()) * distance_factor;
    return std::round(price * 100) / 100;
}

// ============================================================================
// User Operations
// ============================================================================

models::User TaxiStorage::CreateUser(const models::User::CreateRequest& request) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Проверка уникальности логина
    for (const auto& [id, user] : users_) {
        if (user.login == request.login) {
            throw std::runtime_error("User with login '" + request.login + "' already exists");
        }
    }
    
    // Проверка уникальности email
    for (const auto& [id, user] : users_) {
        if (user.email == request.email) {
            throw std::runtime_error("User with email '" + request.email + "' already exists");
        }
    }
    
    models::User user;
    user.id = userver::utils::generators::GenerateUuid();
    user.login = request.login;
    user.first_name = request.first_name;
    user.last_name = request.last_name;
    user.email = request.email;
    user.password_hash = HashPassword(request.password);
    user.created_at = GetCurrentTimestamp();
    
    users_[user.id] = user;
    return user;
}

std::optional<models::User> TaxiStorage::FindUserByLogin(const std::string& login) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [id, user] : users_) {
        if (user.login == login) {
            return user;
        }
    }
    return std::nullopt;
}

std::vector<models::User> TaxiStorage::FindUsersByNameMask(const std::string& mask) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::User> result;
    if (mask.empty()) {
        return result;
    }
    
    for (const auto& [id, user] : users_) {
        std::string full_name = user.first_name + " " + user.last_name;
        std::string full_name_lower = full_name;
        std::string mask_lower = mask;
        
        std::transform(full_name_lower.begin(), full_name_lower.end(), full_name_lower.begin(), ::tolower);
        std::transform(mask_lower.begin(), mask_lower.end(), mask_lower.begin(), ::tolower);
        
        if (full_name_lower.find(mask_lower) != std::string::npos) {
            result.push_back(user);
        }
    }
    return result;
}

std::optional<models::User> TaxiStorage::GetUserById(const std::string& user_id) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = users_.find(user_id);
    if (it != users_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<models::User> TaxiStorage::GetAllUsers() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::User> result;
    result.reserve(users_.size());
    for (const auto& [id, user] : users_) {
        result.push_back(user);
    }
    return result;
}

// ============================================================================
// Driver Operations
// ============================================================================

models::Driver TaxiStorage::CreateDriver(const models::Driver::CreateRequest& request) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Проверка уникальности номера машины
    for (const auto& [id, driver] : drivers_) {
        if (driver.car_number == request.car_number) {
            throw std::runtime_error("Driver with car number '" + request.car_number + "' already exists");
        }
    }
    
    models::Driver driver;
    driver.id = userver::utils::generators::GenerateUuid();
    driver.first_name = request.first_name;
    driver.last_name = request.last_name;
    driver.car_model = request.car_model;
    driver.car_number = request.car_number;
    driver.status = models::DriverStatus::Free;
    driver.rating = 5.0;
    driver.created_at = GetCurrentTimestamp();
    
    drivers_[driver.id] = driver;
    return driver;
}

std::optional<models::Driver> TaxiStorage::GetDriverById(const std::string& driver_id) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = drivers_.find(driver_id);
    if (it != drivers_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool TaxiStorage::UpdateDriverStatus(const std::string& driver_id, models::DriverStatus status) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = drivers_.find(driver_id);
    if (it != drivers_.end()) {
        it->second.status = status;
        return true;
    }
    return false;
}

std::vector<models::Driver> TaxiStorage::GetAllDrivers() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::Driver> result;
    result.reserve(drivers_.size());
    for (const auto& [id, driver] : drivers_) {
        result.push_back(driver);
    }
    return result;
}

std::vector<models::Driver> TaxiStorage::GetAvailableDrivers() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::Driver> result;
    for (const auto& [id, driver] : drivers_) {
        if (driver.status == models::DriverStatus::Free) {
            result.push_back(driver);
        }
    }
    return result;
}

// ============================================================================
// Ride Operations
// ============================================================================

models::Ride TaxiStorage::CreateRide(const models::Ride::CreateRequest& request) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Проверяем существование пользователя
    auto user_it = users_.find(request.user_id);
    if (user_it == users_.end()) {
        throw std::runtime_error("User with id '" + request.user_id + "' not found");
    }
    
    models::Ride ride;
    ride.id = userver::utils::generators::GenerateUuid();
    ride.user_id = request.user_id;
    ride.driver_id = std::nullopt;
    ride.start_address = request.start_address;
    ride.end_address = request.end_address;
    ride.status = models::RideStatus::Created;
    ride.price = CalculateRidePrice(request.start_address, request.end_address);
    ride.created_at = GetCurrentTimestamp();
    ride.accepted_at = std::nullopt;
    ride.completed_at = std::nullopt;
    
    rides_[ride.id] = ride;
    active_rides_.push_back(ride.id);
    
    return ride;
}

std::optional<models::Ride> TaxiStorage::GetRideById(const std::string& ride_id) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = rides_.find(ride_id);
    if (it != rides_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<models::Ride> TaxiStorage::GetActiveRides() {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::Ride> result;
    for (const auto& ride_id : active_rides_) {
        auto it = rides_.find(ride_id);
        if (it != rides_.end() && (it->second.status == models::RideStatus::Created || 
                                   it->second.status == models::RideStatus::Accepted)) {
            result.push_back(it->second);
        }
    }
    return result;
}

std::vector<models::Ride> TaxiStorage::GetUserRideHistory(const std::string& user_id) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<models::Ride> result;
    for (const auto& [id, ride] : rides_) {
        if (ride.user_id == user_id) {
            result.push_back(ride);
        }
    }
    
    // Сортируем по дате создания (от новых к старым)
    std::sort(result.begin(), result.end(), [](const models::Ride& a, const models::Ride& b) {
        return a.created_at > b.created_at;
    });
    
    return result;
}

bool TaxiStorage::AcceptRide(const std::string& ride_id, const std::string& driver_id) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto ride_it = rides_.find(ride_id);
    if (ride_it == rides_.end()) {
        return false;
    }
    
    if (ride_it->second.status != models::RideStatus::Created) {
        return false;
    }
    
    auto driver_it = drivers_.find(driver_id);
    if (driver_it == drivers_.end()) {
        return false;
    }
    
    if (driver_it->second.status != models::DriverStatus::Free) {
        return false;
    }
    
    ride_it->second.driver_id = driver_id;
    ride_it->second.status = models::RideStatus::Accepted;
    ride_it->second.accepted_at = GetCurrentTimestamp();
    
    driver_it->second.status = models::DriverStatus::Busy;
    
    return true;
}

bool TaxiStorage::CompleteRide(const std::string& ride_id) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto ride_it = rides_.find(ride_id);
    if (ride_it == rides_.end()) {
        return false;
    }
    
    if (ride_it->second.status != models::RideStatus::Accepted) {
        return false;
    }
    
    ride_it->second.status = models::RideStatus::Completed;
    ride_it->second.completed_at = GetCurrentTimestamp();
    
    if (ride_it->second.driver_id) {
        auto driver_it = drivers_.find(*ride_it->second.driver_id);
        if (driver_it != drivers_.end()) {
            driver_it->second.status = models::DriverStatus::Free;
        }
    }
    
    active_rides_.erase(
        std::remove(active_rides_.begin(), active_rides_.end(), ride_id),
        active_rides_.end()
    );
    
    return true;
}

bool TaxiStorage::CancelRide(const std::string& ride_id) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto ride_it = rides_.find(ride_id);
    if (ride_it == rides_.end()) {
        return false;
    }
    
    if (ride_it->second.status != models::RideStatus::Created && 
        ride_it->second.status != models::RideStatus::Accepted) {
        return false;
    }
    
    ride_it->second.status = models::RideStatus::Cancelled;
    
    if (ride_it->second.driver_id) {
        auto driver_it = drivers_.find(*ride_it->second.driver_id);
        if (driver_it != drivers_.end()) {
            driver_it->second.status = models::DriverStatus::Free;
        }
    }
    
    active_rides_.erase(
        std::remove(active_rides_.begin(), active_rides_.end(), ride_id),
        active_rides_.end()
    );
    
    return true;
}

// ============================================================================
// Authentication
// ============================================================================

bool TaxiStorage::ValidateUserPassword(const std::string& login, const std::string& password) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [id, user] : users_) {
        if (user.login == login) {
            return user.password_hash == HashPassword(password);
        }
    }
    return false;
}

std::optional<models::User> TaxiStorage::AuthenticateUser(const std::string& login, const std::string& password) {
    if (!ValidateUserPassword(login, password)) {
        return std::nullopt;
    }
    
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    for (const auto& [id, user] : users_) {
        if (user.login == login) {
            models::User user_copy = user;
            user_copy.password_hash = "";
            return user_copy;
        }
    }
    return std::nullopt;
}

// ============================================================================
// Statistics
// ============================================================================

size_t TaxiStorage::GetUsersCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return users_.size();
}

size_t TaxiStorage::GetDriversCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return drivers_.size();
}

size_t TaxiStorage::GetRidesCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return rides_.size();
}

} // namespace storage