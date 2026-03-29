#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <algorithm>
#include <userver/utils/uuid4.hpp>
#include <userver/utils/datetime.hpp>

#include "models/user.hpp"
#include "models/driver.hpp"
#include "models/ride.hpp"

namespace storage {

class TaxiStorage {
public:
    TaxiStorage() = default;
    ~TaxiStorage() = default;
    
    // === User Operations ===
    models::User CreateUser(const models::User::CreateRequest& request);
    std::optional<models::User> FindUserByLogin(const std::string& login);
    std::vector<models::User> FindUsersByNameMask(const std::string& mask);
    std::optional<models::User> GetUserById(const std::string& user_id);
    std::vector<models::User> GetAllUsers();
    
    // === Driver Operations ===
    models::Driver CreateDriver(const models::Driver::CreateRequest& request);
    std::optional<models::Driver> GetDriverById(const std::string& driver_id);
    bool UpdateDriverStatus(const std::string& driver_id, models::DriverStatus status);
    std::vector<models::Driver> GetAllDrivers();
    std::vector<models::Driver> GetAvailableDrivers();
    
    // === Ride Operations ===
    models::Ride CreateRide(const models::Ride::CreateRequest& request);
    std::optional<models::Ride> GetRideById(const std::string& ride_id);
    std::vector<models::Ride> GetActiveRides();
    std::vector<models::Ride> GetUserRideHistory(const std::string& user_id);
    bool AcceptRide(const std::string& ride_id, const std::string& driver_id);
    bool CompleteRide(const std::string& ride_id);
    bool CancelRide(const std::string& ride_id);
    
    // === Authentication ===
    bool ValidateUserPassword(const std::string& login, const std::string& password);
    std::optional<models::User> AuthenticateUser(const std::string& login, const std::string& password);
    
    // === Statistics ===
    size_t GetUsersCount() const;
    size_t GetDriversCount() const;
    size_t GetRidesCount() const;
    
private:
    std::unordered_map<std::string, models::User> users_;
    std::unordered_map<std::string, models::Driver> drivers_;
    std::unordered_map<std::string, models::Ride> rides_;
    std::vector<std::string> active_rides_;
    
    mutable std::shared_mutex mutex_;
    
    std::string HashPassword(const std::string& password);
    std::string GetCurrentTimestamp();
    double CalculateRidePrice(const std::string& start_address, const std::string& end_address);
};

} // namespace storage