#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <userver/storages/postgres/cluster.hpp>
#include "../models/driver.hpp"

namespace db {

class DriverRepository {
public:
    explicit DriverRepository(const userver::storages::postgres::ClusterPtr& pg_cluster);
    
    // Создание водителя
    models::Driver CreateDriver(const models::Driver::CreateRequest& request);
    
    // Поиск по ID
    std::optional<models::Driver> FindDriverById(const std::string& driver_id);
    
    // Обновление статуса
    bool UpdateDriverStatus(const std::string& driver_id, models::DriverStatus status);
    
    // Поиск свободных водителей
    std::vector<models::Driver> GetAvailableDrivers();
    
    // Все водители
    std::vector<models::Driver> GetAllDrivers();
    
private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace db