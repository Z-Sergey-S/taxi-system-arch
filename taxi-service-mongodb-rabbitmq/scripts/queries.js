// MongoDB ЗАПРОСЫ ДЛЯ ВСЕХ ОПЕРАЦИЙ API
// Система заказа такси

// Подключаемся к базе данных
db = db.getSiblingDB('taxi_db');

print("=== MongoDB QUERIES FOR TAXI SYSTEM ===\n");


// 1. Создание нового пользователя (CREATE)

print("1. CREATE USER:");
printjson(
    db.users.insertOne({
        login: "new_user",
        first_name: "New",
        last_name: "User",
        email: "new@example.com",
        password_hash: "hash_password123",
        phone: "+1234567890",
        rating: 5.0,
        created_at: new Date()
    })
);


// 2. Поиск пользователя по логину (READ - $eq)

print("\n2. FIND USER BY LOGIN (john_doe):");
printjson(
    db.users.findOne({ login: "john_doe" })
);


// 3. Поиск пользователя по маске имени и фамилии (READ - $regex)

print("\n3. SEARCH USERS BY NAME MASK (Jo, Do):");
printjson(
    db.users.find({
        first_name: { $regex: "Jo", $options: "i" },
        last_name: { $regex: "Do", $options: "i" }
    }).toArray()
);


// 4. Регистрация водителя (CREATE)

print("\n4. REGISTER DRIVER:");
printjson(
    db.drivers.insertOne({
        first_name: "New",
        last_name: "Driver",
        car_model: "Tesla Model 3",
        car_number: "X999XX",
        status: "free",
        rating: 5.0,
        location: { type: "Point", coordinates: [37.6176, 55.7558] },
        created_at: new Date()
    })
);


// 5. Создание заказа поездки (CREATE)

print("\n5. CREATE RIDE:");

// Находим пользователя
var user = db.users.findOne({ login: "john_doe" });
if (user) {
    var rideResult = db.rides.insertOne({
        user_id: user._id,
        start_address: "ул. Ленина, 10",
        end_address: "ул. Пушкина, 20",
        status: "created",
        price: 510.00,
        timestamps: {
            created_at: new Date()
        }
    });
    printjson(rideResult);
    
    // Обновляем активную поездку у пользователя (embedded)
    db.users.updateOne(
        { _id: user._id },
        { $set: {
            active_ride: {
                ride_id: rideResult.insertedId,
                status: "created",
                start_address: "ул. Ленина, 10",
                end_address: "ул. Пушкина, 20",
                created_at: new Date()
            }
        }}
    );
}


// 6. Получение активных заказов (READ - $in)

print("\n6. GET ACTIVE RIDES:");
printjson(
    db.rides.find({
        status: { $in: ["created", "accepted"] }
    }).sort({ "timestamps.created_at": 1 }).toArray()
);


// 7. Принятие заказа водителем (UPDATE - $set)

print("\n7. ACCEPT RIDE BY DRIVER:");

// Находим свободного водителя
var driver = db.drivers.findOne({ status: "free" });
// Находим активную поездку
var ride = db.rides.findOne({ status: "created" });

if (driver && ride) {
    // Обновляем поездку
    var acceptResult = db.rides.updateOne(
        { _id: ride._id, status: "created" },
        { $set: {
            driver_id: driver._id,
            status: "accepted",
            "timestamps.accepted_at": new Date()
        }}
    );
    printjson(acceptResult);
    
    // Обновляем статус водителя
    db.drivers.updateOne(
        { _id: driver._id },
        { $set: { status: "busy" } }
    );
    
    // Обновляем текущую поездку водителя (embedded)
    var user = db.users.findOne({ _id: ride.user_id });
    db.drivers.updateOne(
        { _id: driver._id },
        { $set: {
            current_ride: {
                ride_id: ride._id,
                user_id: ride.user_id,
                user_name: user.first_name + " " + user.last_name,
                start_address: ride.start_address,
                end_address: ride.end_address,
                accepted_at: new Date()
            }
        }}
    );
    
    print("Ride accepted successfully!");
}


// 8. Получение истории поездок пользователя (READ - $and, $or)

print("\n8. GET USER RIDE HISTORY:");
if (user) {
    printjson(
        db.rides.find({ user_id: user._id })
            .sort({ "timestamps.created_at": -1 })
            .toArray()
    );
}


// 9. Завершение поездки (UPDATE - $set, $unset)

print("\n9. COMPLETE RIDE:");

// Находим принятую поездку
var activeRide = db.rides.findOne({ status: "accepted" });

if (activeRide && activeRide.driver_id) {
    // Завершаем поездку
    var completeResult = db.rides.updateOne(
        { _id: activeRide._id, status: "accepted" },
        { $set: {
            status: "completed",
            "timestamps.completed_at": new Date()
        }}
    );
    printjson(completeResult);
    
    // Освобождаем водителя
    db.drivers.updateOne(
        { _id: activeRide.driver_id },
        { 
            $set: { status: "free" },
            $unset: { current_ride: "" }
        }
    );
    
    // Очищаем активную поездку у пользователя
    db.users.updateOne(
        { _id: activeRide.user_id },
        { $unset: { active_ride: "" } }
    );
    
    print("Ride completed successfully!");
}


// 10. Аутентификация пользователя (READ - $eq)

print("\n10. AUTHENTICATE USER (john_doe):");
var authUser = db.users.findOne({ login: "john_doe" });
if (authUser) {
    printjson({
        id: authUser._id,
        login: authUser.login,
        first_name: authUser.first_name,
        last_name: authUser.last_name,
        email: authUser.email,
        hashed_password: authUser.password_hash
    });
}


// ДОПОЛНИТЕЛЬНЫЕ ЗАПРОСЫ С РАЗЛИЧНЫМИ ОПЕРАТОРАМИ


// 11. Поиск водителей с рейтингом выше 4.5 ($gt)
print("\n11. DRIVERS WITH RATING > 4.5 ($gt):");
printjson(
    db.drivers.find({ rating: { $gt: 4.5 } })
        .sort({ rating: -1 })
        .toArray()
);

// 12. Поиск водителей с рейтингом между 4.0 и 4.8 ($and, $gte, $lte)
print("\n12. DRIVERS WITH RATING BETWEEN 4.0 AND 4.8 ($and, $gte, $lte):");
printjson(
    db.drivers.find({
        $and: [
            { rating: { $gte: 4.0 } },
            { rating: { $lte: 4.8 } }
        ]
    }).toArray()
);

// 13. Поиск завершенных поездок с ценой больше 400 ($gt)
print("\n13. COMPLETED RIDES WITH PRICE > 400 ($gt):");
printjson(
    db.rides.find({ 
        status: "completed", 
        price: { $gt: 400 } 
    }).toArray()
);

// 14. Обновление рейтинга водителя после поездки ($inc)
print("\n14. UPDATE DRIVER RATING AFTER RIDE ($inc):");
if (activeRide && activeRide.driver_id) {
    var updateRating = db.drivers.updateOne(
        { _id: activeRide.driver_id },
        { $inc: { rating: 0.1 } }
    );
    printjson(updateRating);
}

// 15. Добавление телефона пользователю, если его нет ($exists)
print("\n15. UPDATE USERS WITHOUT PHONE ($exists):");
var updatePhone = db.users.updateMany(
    { phone: { $exists: false } },
    { $set: { phone: "+0000000000" } }
);
printjson(updatePhone);

// 16. Удаление отмененных поездок (DELETE)
print("\n16. DELETE CANCELLED RIDES:");
var deleteResult = db.rides.deleteMany({ status: "cancelled" });
printjson(deleteResult);

// 17. Поиск поездок пользователя с определенным статусом ($and)
print("\n17. USER RIDES WITH SPECIFIC STATUS ($and):");
if (user) {
    printjson(
        db.rides.find({
            $and: [
                { user_id: user._id },
                { status: "completed" }
            ]
        }).toArray()
    );
}

// 18. Операции с массивами ($push, $pull, $addToSet)
print("\n18. ARRAY OPERATIONS EXAMPLE:");

// Добавляем историю поездок в массив пользователя
// Вместо отдельной коллекции rides, можно хранить массив references
db.users.updateOne(
    { login: "john_doe" },
    { $push: { ride_ids: "example_ride_id" } }
);
print("Added ride_id to user's ride_ids array");

// Удаляем из массива
db.users.updateOne(
    { login: "john_doe" },
    { $pull: { ride_ids: "example_ride_id" } }
);
print("Removed ride_id from user's ride_ids array");

// Добавляем уникальное значение в массив
db.users.updateOne(
    { login: "john_doe" },
    { $addToSet: { tags: "premium" } }
);
print("Added unique tag to user");

print("\n=== ALL QUERIES EXECUTED SUCCESSFULLY ===");