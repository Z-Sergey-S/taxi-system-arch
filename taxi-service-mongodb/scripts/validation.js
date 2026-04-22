// ТЕСТИРОВАНИЕ ВАЛИДАЦИИ СХЕМ MongoDB
// Система заказа такси

db = db.getSiblingDB('taxi_db');

print("=== MONGODB SCHEMA VALIDATION TESTS ===\n");


// 1. ТЕСТИРОВАНИЕ ВАЛИДАЦИИ ПОЛЬЗОВАТЕЛЕЙ

print("1. VALIDATING USERS COLLECTION:");

// 1.1 Валидный пользователь (должен успешно вставиться)
print("\n1.1 Valid user (should succeed):");
try {
    var validUser = {
        login: "valid_user",
        first_name: "Valid",
        last_name: "User",
        email: "valid@example.com",
        password_hash: "hash_valid123",
        created_at: new Date()
    };
    var result = db.users.insertOne(validUser);
    print("✓ SUCCESS: Valid user inserted, id: " + result.insertedId);
} catch (e) {
    print("✗ ERROR: " + e);
}

// 1.2 Невалидный пользователь - отсутствует обязательное поле login
print("\n1.2 Invalid user - missing login (should fail):");
try {
    var invalidUser1 = {
        first_name: "Invalid",
        last_name: "User",
        email: "invalid@example.com",
        password_hash: "hash_invalid",
        created_at: new Date()
    };
    db.users.insertOne(invalidUser1);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 1.3 Невалидный пользователь - слишком короткий login
print("\n1.3 Invalid user - login too short (minLength: 3):");
try {
    var invalidUser2 = {
        login: "ab",
        first_name: "Invalid",
        last_name: "User",
        email: "invalid@example.com",
        password_hash: "hash_invalid",
        created_at: new Date()
    };
    db.users.insertOne(invalidUser2);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 1.4 Невалидный пользователь - некорректный email
print("\n1.4 Invalid user - invalid email format:");
try {
    var invalidUser3 = {
        login: "invalid_user",
        first_name: "Invalid",
        last_name: "User",
        email: "not-an-email",
        password_hash: "hash_invalid",
        created_at: new Date()
    };
    db.users.insertOne(invalidUser3);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 1.5 Невалидный пользователь - рейтинг вне диапазона
print("\n1.5 Invalid user - rating out of range (0-5):");
try {
    var invalidUser4 = {
        login: "invalid_rating",
        first_name: "Invalid",
        last_name: "User",
        email: "rating@example.com",
        password_hash: "hash_invalid",
        rating: 10.0,
        created_at: new Date()
    };
    db.users.insertOne(invalidUser4);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}



// 2. ТЕСТИРОВАНИЕ ВАЛИДАЦИИ ВОДИТЕЛЕЙ

print("\n2. VALIDATING DRIVERS COLLECTION:");

// 2.1 Валидный водитель (должен успешно вставиться)
print("\n2.1 Valid driver (should succeed):");
try {
    var validDriver = {
        first_name: "Valid",
        last_name: "Driver",
        car_model: "Toyota Camry",
        car_number: "V999VV",
        status: "free",
        created_at: new Date()
    };
    var result = db.drivers.insertOne(validDriver);
    print("✓ SUCCESS: Valid driver inserted, id: " + result.insertedId);
} catch (e) {
    print("✗ ERROR: " + e);
}

// 2.2 Невалидный водитель - отсутствует обязательное поле
print("\n2.2 Invalid driver - missing first_name (should fail):");
try {
    var invalidDriver1 = {
        last_name: "Driver",
        car_model: "Toyota",
        car_number: "X123XX",
        status: "free",
        created_at: new Date()
    };
    db.drivers.insertOne(invalidDriver1);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 2.3 Невалидный водитель - некорректный статус
print("\n2.3 Invalid driver - invalid status (enum: free, busy, offline):");
try {
    var invalidDriver2 = {
        first_name: "Invalid",
        last_name: "Driver",
        car_model: "Toyota",
        car_number: "X456XX",
        status: "invalid_status",
        created_at: new Date()
    };
    db.drivers.insertOne(invalidDriver2);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 2.4 Невалидный водитель - рейтинг вне диапазона
print("\n2.4 Invalid driver - rating out of range (0-5):");
try {
    var invalidDriver3 = {
        first_name: "Invalid",
        last_name: "Driver",
        car_model: "Toyota",
        car_number: "X789XX",
        status: "free",
        rating: 6.0,
        created_at: new Date()
    };
    db.drivers.insertOne(invalidDriver3);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}

// 2.5 Невалидный водитель - некорректный номер авто
print("\n2.5 Invalid driver - invalid car_number pattern:");
try {
    var invalidDriver4 = {
        first_name: "Invalid",
        last_name: "Driver",
        car_model: "Toyota",
        car_number: "invalid!!!",
        status: "free",
        created_at: new Date()
    };
    db.drivers.insertOne(invalidDriver4);
    print("✗ ERROR: Should have failed but didn't!");
} catch (e) {
    print("✓ SUCCESS: Validation failed as expected: " + e.message);
}



// 3. ТЕСТИРОВАНИЕ УНИКАЛЬНЫХ ИНДЕКСОВ

print("\n3. TESTING UNIQUE INDEXES:");

// 3.1 Дубликат логина
print("\n3.1 Duplicate login (should fail):");
try {
    var duplicateUser = {
        login: "john_doe",
        first_name: "Duplicate",
        last_name: "User",
        email: "duplicate@example.com",
        password_hash: "hash_dup",
        created_at: new Date()
    };
    db.users.insertOne(duplicateUser);
    print("✗ ERROR: Duplicate login should have failed!");
} catch (e) {
    print("✓ SUCCESS: Duplicate login rejected: " + e.message);
}

// 3.2 Дубликат номера авто
print("\n3.2 Duplicate car_number (should fail):");
try {
    var duplicateDriver = {
        first_name: "Duplicate",
        last_name: "Driver",
        car_model: "Toyota",
        car_number: "A123BC",
        status: "free",
        created_at: new Date()
    };
    db.drivers.insertOne(duplicateDriver);
    print("✗ ERROR: Duplicate car_number should have failed!");
} catch (e) {
    print("✓ SUCCESS: Duplicate car_number rejected: " + e.message);
}



// 4. ТЕСТИРОВАНИЕ EMBEDDED ДОКУМЕНТОВ

print("\n4. TESTING EMBEDDED DOCUMENTS:");

// 4.1 Добавление активной поездки пользователю
print("\n4.1 Adding active_ride to user (embedded):");
try {
    var user = db.users.findOne({ login: "john_doe" });
    if (user) {
        var result = db.users.updateOne(
            { _id: user._id },
            { $set: {
                active_ride: {
                    ride_id: ObjectId(),
                    status: "created",
                    start_address: "Test Start",
                    end_address: "Test End",
                    created_at: new Date()
                }
            }}
        );
        print("✓ SUCCESS: Active ride added: " + JSON.stringify(result));
    }
} catch (e) {
    print("✗ ERROR: " + e.message);
}

// 4.2 Проверка активной поездки
print("\n4.2 Checking active_ride in user document:");
var userWithRide = db.users.findOne({ login: "john_doe" });
if (userWithRide && userWithRide.active_ride) {
    print("✓ Active ride found: " + JSON.stringify(userWithRide.active_ride));
} else {
    print("No active ride found");
}

// 4.3 Очистка активной поездки
print("\n4.3 Clearing active_ride from user:");
try {
    var user = db.users.findOne({ login: "john_doe" });
    if (user && user.active_ride) {
        var result = db.users.updateOne(
            { _id: user._id },
            { $unset: { active_ride: "" } }
        );
        print("✓ SUCCESS: Active ride cleared: " + JSON.stringify(result));
    }
} catch (e) {
    print("✗ ERROR: " + e.message);
}



// 5. ТЕСТИРОВАНИЕ GEOJSON

print("\n5. TESTING GEOJSON (2dsphere index):");

// 5.1 Вставка водителя с геолокацией
print("\n5.1 Insert driver with GeoJSON location:");
try {
    var geoDriver = {
        first_name: "Geo",
        last_name: "Driver",
        car_model: "Tesla",
        car_number: "GEO123",
        status: "free",
        rating: 5.0,
        location: {
            type: "Point",
            coordinates: [37.6176, 55.7558]
        },
        created_at: new Date()
    };
    var result = db.drivers.insertOne(geoDriver);
    print("✓ SUCCESS: Geo driver inserted: " + result.insertedId);
} catch (e) {
    print("✗ ERROR: " + e.message);
}

// 5.2 Поиск ближайших водителей
print("\n5.2 Find nearby drivers (geospatial query):");
try {
    var nearbyDrivers = db.drivers.find({
        location: {
            $near: {
                $geometry: {
                    type: "Point",
                    coordinates: [37.6176, 55.7558]
                },
                $maxDistance: 5000
            }
        }
    }).limit(5).toArray();
    print("✓ Found " + nearbyDrivers.length + " nearby drivers");
    printjson(nearbyDrivers);
} catch (e) {
    print("✗ ERROR: " + e.message);
}



// 6. ТЕСТИРОВАНИЕ АГРЕГАЦИЙ

print("\n6. TESTING AGGREGATIONS:");

// 6.1 Статистика по поездкам пользователя
print("\n6.1 User ride statistics:");
try {
    var user = db.users.findOne({ login: "john_doe" });
    if (user) {
        var stats = db.rides.aggregate([
            { $match: { user_id: user._id } },
            { $group: {
                _id: "$user_id",
                total_rides: { $sum: 1 },
                completed_rides: { $sum: { $cond: [{ $eq: ["$status", "completed"] }, 1, 0] } },
                total_spent: { $sum: { $cond: [{ $eq: ["$status", "completed"] }, "$price", 0] } },
                avg_price: { $avg: "$price" }
            }}
        ]).toArray();
        printjson(stats);
    }
} catch (e) {
    print("✗ ERROR: " + e.message);
}

// 6.2 Топ водителей по количеству поездок
print("\n6.2 Top drivers by ride count:");
try {
    var topDrivers = db.rides.aggregate([
        { $match: { driver_id: { $exists: true } } },
        { $group: {
            _id: "$driver_id",
            total_rides: { $sum: 1 },
            total_earnings: { $sum: "$price" }
        }},
        { $sort: { total_rides: -1 } },
        { $limit: 5 },
        { $lookup: {
            from: "drivers",
            localField: "_id",
            foreignField: "_id",
            as: "driver_info"
        }},
        { $unwind: "$driver_info" },
        { $project: {
            driver_name: { $concat: ["$driver_info.first_name", " ", "$driver_info.last_name"] },
            car_model: "$driver_info.car_model",
            total_rides: 1,
            total_earnings: 1
        }}
    ]).toArray();
    printjson(topDrivers);
} catch (e) {
    print("✗ ERROR: " + e.message);
}

print("\n=== VALIDATION TESTS COMPLETED ===");