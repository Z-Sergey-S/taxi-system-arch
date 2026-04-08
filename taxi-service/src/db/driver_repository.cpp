#include "driver_repository.hpp"
#include <userver/storages/postgres/io/uuid.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace db {

DriverRepository::DriverRepository(const userver::storages::postgres::ClusterPtr& pg_cluster)
    : pg_cluster_(pg_cluster) {}

models::Driver DriverRepository::CreateDriver(const models::Driver::CreateRequest& request) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            INSERT INTO taxi_schema.drivers (first_name, last_name, car_model, car_number)
            VALUES ($1, $2, $3, $4)
            RETURNING id::text, first_name, last_name, car_model, car_number, status, rating, created_at::text
        )",
        request.first_name, request.last_name, request.car_model, request.car_number
    );
    
    if (result.IsEmpty()) {
        throw std::runtime_error("Failed to create driver");
    }
    
    auto row = result.Front();
    models::Driver driver;
    driver.id = row["id"].As<std::string>();
    driver.first_name = row["first_name"].As<std::string>();
    driver.last_name = row["last_name"].As<std::string>();
    driver.car_model = row["car_model"].As<std::string>();
    driver.car_number = row["car_number"].As<std::string>();
    
    std::string status_str = row["status"].As<std::string>();
    if (status_str == "free") driver.status = models::DriverStatus::Free;
    else if (status_str == "busy") driver.status = models::DriverStatus::Busy;
    else driver.status = models::DriverStatus::Offline;
    
    driver.rating = row["rating"].As<double>();
    driver.created_at = row["created_at"].As<std::string>();
    
    return driver;
}

std::optional<models::Driver> DriverRepository::FindDriverById(const std::string& driver_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, first_name, last_name, car_model, car_number, status, rating, created_at::text
            FROM taxi_schema.drivers
            WHERE id = $1::uuid
        )",
        driver_id
    );
    
    if (result.IsEmpty()) {
        return std::nullopt;
    }
    
    auto row = result.Front();
    models::Driver driver;
    driver.id = row["id"].As<std::string>();
    driver.first_name = row["first_name"].As<std::string>();
    driver.last_name = row["last_name"].As<std::string>();
    driver.car_model = row["car_model"].As<std::string>();
    driver.car_number = row["car_number"].As<std::string>();
    
    std::string status_str = row["status"].As<std::string>();
    if (status_str == "free") driver.status = models::DriverStatus::Free;
    else if (status_str == "busy") driver.status = models::DriverStatus::Busy;
    else driver.status = models::DriverStatus::Offline;
    
    driver.rating = row["rating"].As<double>();
    driver.created_at = row["created_at"].As<std::string>();
    
    return driver;
}

bool DriverRepository::UpdateDriverStatus(const std::string& driver_id, models::DriverStatus status) {
    std::string status_str;
    switch (status) {
        case models::DriverStatus::Free: status_str = "free"; break;
        case models::DriverStatus::Busy: status_str = "busy"; break;
        default: status_str = "offline"; break;
    }
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE taxi_schema.drivers
            SET status = $2
            WHERE id = $1::uuid
            RETURNING id
        )",
        driver_id, status_str
    );
    
    return !result.IsEmpty();
}

std::vector<models::Driver> DriverRepository::GetAvailableDrivers() {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, first_name, last_name, car_model, car_number, status, rating, created_at::text
            FROM taxi_schema.drivers
            WHERE status = 'free'
            ORDER BY rating DESC
        )"
    );
    
    std::vector<models::Driver> drivers;
    for (const auto& row : result) {
        models::Driver driver;
        driver.id = row["id"].As<std::string>();
        driver.first_name = row["first_name"].As<std::string>();
        driver.last_name = row["last_name"].As<std::string>();
        driver.car_model = row["car_model"].As<std::string>();
        driver.car_number = row["car_number"].As<std::string>();
        driver.status = models::DriverStatus::Free;
        driver.rating = row["rating"].As<double>();
        driver.created_at = row["created_at"].As<std::string>();
        drivers.push_back(driver);
    }
    
    return drivers;
}

std::vector<models::Driver> DriverRepository::GetAllDrivers() {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, first_name, last_name, car_model, car_number, status, rating, created_at::text
            FROM taxi_schema.drivers
            ORDER BY created_at DESC
        )"
    );
    
    std::vector<models::Driver> drivers;
    for (const auto& row : result) {
        models::Driver driver;
        driver.id = row["id"].As<std::string>();
        driver.first_name = row["first_name"].As<std::string>();
        driver.last_name = row["last_name"].As<std::string>();
        driver.car_model = row["car_model"].As<std::string>();
        driver.car_number = row["car_number"].As<std::string>();
        
        std::string status_str = row["status"].As<std::string>();
        if (status_str == "free") driver.status = models::DriverStatus::Free;
        else if (status_str == "busy") driver.status = models::DriverStatus::Busy;
        else driver.status = models::DriverStatus::Offline;
        
        driver.rating = row["rating"].As<double>();
        driver.created_at = row["created_at"].As<std::string>();
        drivers.push_back(driver);
    }
    
    return drivers;
}

} // namespace db