#include <gtest/gtest.h>
#include "../storage/taxi_storage.hpp"

TEST(TaxiStorageTest, CreateAndFindUser) {
    storage::TaxiStorage storage;
    
    models::User::CreateRequest request;
    request.login = "test_user";
    request.password = "pass";
    request.first_name = "Test";
    request.last_name = "User";
    request.email = "test@example.com";
    
    auto user = storage.CreateUser(request);
    EXPECT_EQ(user.login, "test_user");
    
    auto found = storage.FindUserByLogin("test_user");
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->login, "test_user");
}

TEST(TaxiStorageTest, CreateRide) {
    storage::TaxiStorage storage;
    
    models::User::CreateRequest user_request;
    user_request.login = "rider";
    user_request.password = "pass";
    user_request.first_name = "Rider";
    user_request.last_name = "Test";
    user_request.email = "rider@example.com";
    auto user = storage.CreateUser(user_request);
    
    models::Ride::CreateRequest ride_request;
    ride_request.user_id = user.id;
    ride_request.start_address = "Start";
    ride_request.end_address = "End";
    
    auto ride = storage.CreateRide(ride_request);
    EXPECT_EQ(ride.user_id, user.id);
    EXPECT_EQ(ride.status, models::RideStatus::Created);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}