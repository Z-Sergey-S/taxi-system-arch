#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <userver/storages/postgres/cluster.hpp>
#include "../models/ride.hpp"

namespace db {

class RideRepository {
public:
    explicit RideRepository(const userver::storages::postgres::ClusterPtr& pg_cluster);
    
    // Создание поездки
    models::Ride CreateRide(const models::Ride::CreateRequest& request);
    
    // Поиск по ID
    std::optional<models::Ride> FindRideById(const std::string& ride_id);
    
    // Активные поездки
    std::vector<models::Ride> GetActiveRides();
    
    // История поездок пользователя
    std::vector<models::Ride> GetUserRideHistory(const std::string& user_id);
    
    // Принятие заказа
    bool AcceptRide(const std::string& ride_id, const std::string& driver_id);
    
    // Завершение поездки
    bool CompleteRide(const std::string& ride_id);
    
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
    
    double CalculateRidePrice(const std::string& start_address, const std::string& end_address);
};

} // namespace db