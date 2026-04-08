#include "ride_repository.hpp"
#include <userver/storages/postgres/io/uuid.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <cmath>

namespace db {

RideRepository::RideRepository(const userver::storages::postgres::ClusterPtr& pg_cluster)
    : pg_cluster_(pg_cluster) {}

double RideRepository::CalculateRidePrice(const std::string& start_address, const std::string& end_address) {
    double base_price = 50.0;
    double distance_factor = 10.0;
    double price = base_price + (start_address.length() + end_address.length()) * distance_factor;
    return std::round(price * 100) / 100;
}

models::Ride RideRepository::CreateRide(const models::Ride::CreateRequest& request) {
    double price = CalculateRidePrice(request.start_address, request.end_address);
    
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            INSERT INTO taxi_schema.rides (user_id, start_address, end_address, price)
            VALUES ($1::uuid, $2, $3, $4)
            RETURNING id::text, user_id::text, start_address, end_address, status, price, created_at::text
        )",
        request.user_id, request.start_address, request.end_address, price
    );
    
    if (result.IsEmpty()) {
        throw std::runtime_error("Failed to create ride");
    }
    
    auto row = result.Front();
    models::Ride ride;
    ride.id = row["id"].As<std::string>();
    ride.user_id = row["user_id"].As<std::string>();
    ride.start_address = row["start_address"].As<std::string>();
    ride.end_address = row["end_address"].As<std::string>();
    
    std::string status_str = row["status"].As<std::string>();
    if (status_str == "created") ride.status = models::RideStatus::Created;
    else if (status_str == "accepted") ride.status = models::RideStatus::Accepted;
    else if (status_str == "completed") ride.status = models::RideStatus::Completed;
    else ride.status = models::RideStatus::Cancelled;
    
    ride.price = row["price"].As<double>();
    ride.created_at = row["created_at"].As<std::string>();
    
    return ride;
}

std::optional<models::Ride> RideRepository::FindRideById(const std::string& ride_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, user_id::text, driver_id::text, start_address, end_address, status, price, 
                   created_at::text, accepted_at::text, completed_at::text
            FROM taxi_schema.rides
            WHERE id = $1::uuid
        )",
        ride_id
    );
    
    if (result.IsEmpty()) {
        return std::nullopt;
    }
    
    auto row = result.Front();
    models::Ride ride;
    ride.id = row["id"].As<std::string>();
    ride.user_id = row["user_id"].As<std::string>();
    ride.driver_id = row["driver_id"].IsNull() ? std::nullopt : 
                     std::make_optional(row["driver_id"].As<std::string>());
    ride.start_address = row["start_address"].As<std::string>();
    ride.end_address = row["end_address"].As<std::string>();
    
    std::string status_str = row["status"].As<std::string>();
    if (status_str == "created") ride.status = models::RideStatus::Created;
    else if (status_str == "accepted") ride.status = models::RideStatus::Accepted;
    else if (status_str == "completed") ride.status = models::RideStatus::Completed;
    else ride.status = models::RideStatus::Cancelled;
    
    ride.price = row["price"].As<double>();
    ride.created_at = row["created_at"].As<std::string>();
    ride.accepted_at = row["accepted_at"].IsNull() ? std::nullopt : 
                       std::make_optional(row["accepted_at"].As<std::string>());
    ride.completed_at = row["completed_at"].IsNull() ? std::nullopt : 
                        std::make_optional(row["completed_at"].As<std::string>());
    
    return ride;
}

std::vector<models::Ride> RideRepository::GetActiveRides() {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, user_id::text, driver_id::text, start_address, end_address, status, price, 
                   created_at::text, accepted_at::text, completed_at::text
            FROM taxi_schema.rides
            WHERE status IN ('created', 'accepted')
            ORDER BY created_at ASC
        )"
    );
    
    std::vector<models::Ride> rides;
    for (const auto& row : result) {
        models::Ride ride;
        ride.id = row["id"].As<std::string>();
        ride.user_id = row["user_id"].As<std::string>();
        ride.driver_id = row["driver_id"].IsNull() ? std::nullopt : 
                         std::make_optional(row["driver_id"].As<std::string>());
        ride.start_address = row["start_address"].As<std::string>();
        ride.end_address = row["end_address"].As<std::string>();
        
        std::string status_str = row["status"].As<std::string>();
        if (status_str == "created") ride.status = models::RideStatus::Created;
        else if (status_str == "accepted") ride.status = models::RideStatus::Accepted;
        else ride.status = models::RideStatus::Cancelled;
        
        ride.price = row["price"].As<double>();
        ride.created_at = row["created_at"].As<std::string>();
        ride.accepted_at = row["accepted_at"].IsNull() ? std::nullopt : 
                           std::make_optional(row["accepted_at"].As<std::string>());
        ride.completed_at = row["completed_at"].IsNull() ? std::nullopt : 
                            std::make_optional(row["completed_at"].As<std::string>());
        rides.push_back(ride);
    }
    
    return rides;
}

std::vector<models::Ride> RideRepository::GetUserRideHistory(const std::string& user_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        R"(
            SELECT id::text, user_id::text, driver_id::text, start_address, end_address, status, price, 
                   created_at::text, accepted_at::text, completed_at::text
            FROM taxi_schema.rides
            WHERE user_id = $1::uuid
            ORDER BY created_at DESC
        )",
        user_id
    );
    
    std::vector<models::Ride> rides;
    for (const auto& row : result) {
        models::Ride ride;
        ride.id = row["id"].As<std::string>();
        ride.user_id = row["user_id"].As<std::string>();
        ride.driver_id = row["driver_id"].IsNull() ? std::nullopt : 
                         std::make_optional(row["driver_id"].As<std::string>());
        ride.start_address = row["start_address"].As<std::string>();
        ride.end_address = row["end_address"].As<std::string>();
        
        std::string status_str = row["status"].As<std::string>();
        if (status_str == "created") ride.status = models::RideStatus::Created;
        else if (status_str == "accepted") ride.status = models::RideStatus::Accepted;
        else if (status_str == "completed") ride.status = models::RideStatus::Completed;
        else ride.status = models::RideStatus::Cancelled;
        
        ride.price = row["price"].As<double>();
        ride.created_at = row["created_at"].As<std::string>();
        ride.accepted_at = row["accepted_at"].IsNull() ? std::nullopt : 
                           std::make_optional(row["accepted_at"].As<std::string>());
        ride.completed_at = row["completed_at"].IsNull() ? std::nullopt : 
                            std::make_optional(row["completed_at"].As<std::string>());
        rides.push_back(ride);
    }
    
    return rides;
}

bool RideRepository::AcceptRide(const std::string& ride_id, const std::string& driver_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE taxi_schema.rides
            SET driver_id = $2::uuid,
                status = 'accepted',
                accepted_at = CURRENT_TIMESTAMP
            WHERE id = $1::uuid AND status = 'created'
            RETURNING id
        )",
        ride_id, driver_id
    );
    
    if (result.IsEmpty()) {
        return false;
    }
    
    pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE taxi_schema.drivers
            SET status = 'busy'
            WHERE id = $1::uuid
        )",
        driver_id
    );
    
    return true;
}

bool RideRepository::CompleteRide(const std::string& ride_id) {
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE taxi_schema.rides
            SET status = 'completed',
                completed_at = CURRENT_TIMESTAMP
            WHERE id = $1::uuid AND status = 'accepted'
            RETURNING driver_id::text
        )",
        ride_id
    );
    
    if (result.IsEmpty()) {
        return false;
    }
    
    auto row = result.Front();
    std::string driver_id = row["driver_id"].As<std::string>();
    
    pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        R"(
            UPDATE taxi_schema.drivers
            SET status = 'free'
            WHERE id = $1::uuid
        )",
        driver_id
    );
    
    return true;
}

} // namespace db