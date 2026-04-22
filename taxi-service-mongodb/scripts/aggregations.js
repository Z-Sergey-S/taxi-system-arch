// MONGODB AGGREGATION PIPELINES
// Система заказа такси

db = db.getSiblingDB('taxi_db');

print("=== MONGODB AGGREGATION PIPELINES ===\n");

// АГРЕГАЦИЯ 1: Статистика по пользователю
print("1. USER STATISTICS AGGREGATION:");

var userStats = db.rides.aggregate([
    // Стадия 1: Фильтрация - только завершенные поездки
    {
        $match: {
            status: "completed"
        }
    },
    
    // Стадия 2: Группировка по user_id
    {
        $group: {
            _id: "$user_id",
            total_rides: { $sum: 1 },
            total_spent: { $sum: "$price" },
            avg_price: { $avg: "$price" },
            min_price: { $min: "$price" },
            max_price: { $max: "$price" }
        }
    },
    
    // Стадия 3: Сортировка по убыванию потраченной суммы
    {
        $sort: { total_spent: -1 }
    },
    
    // Стадия 4: Ограничение количества результатов (топ-10)
    {
        $limit: 10
    },
    
    // Стадия 5: JOIN с коллекцией users для получения информации о пользователе
    {
        $lookup: {
            from: "users",
            localField: "_id",
            foreignField: "_id",
            as: "user_info"
        }
    },
    
    // Стадия 6: Разворачивание массива user_info
    {
        $unwind: {
            path: "$user_info",
            preserveNullAndEmptyArrays: false
        }
    },
    
    // Стадия 7: Формирование выходных данных
    {
        $project: {
            _id: 0,
            user_id: "$_id",
            user_name: { $concat: ["$user_info.first_name", " ", "$user_info.last_name"] },
            user_login: "$user_info.login",
            total_rides: 1,
            total_spent: { $round: ["$total_spent", 2] },
            avg_price: { $round: ["$avg_price", 2] },
            min_price: { $round: ["$min_price", 2] },
            max_price: { $round: ["$max_price", 2] }
        }
    }
]).toArray();

printjson(userStats);

// АГРЕГАЦИЯ 2: Статистика по водителям
print("\n2. DRIVER STATISTICS AGGREGATION:");

var driverStats = db.rides.aggregate([
    // Стадия 1: Фильтрация - только поездки с водителем
    {
        $match: {
            driver_id: { $exists: true, $ne: null },
            status: "completed"
        }
    },
    
    // Стадия 2: Группировка по driver_id
    {
        $group: {
            _id: "$driver_id",
            total_rides: { $sum: 1 },
            total_earnings: { $sum: "$price" },
            avg_price: { $avg: "$price" }
        }
    },
    
    // Стадия 3: JOIN с коллекцией drivers
    {
        $lookup: {
            from: "drivers",
            localField: "_id",
            foreignField: "_id",
            as: "driver_info"
        }
    },
    
    // Стадия 4: Разворачивание массива
    {
        $unwind: "$driver_info"
    },
    
    // Стадия 5: Проекция
    {
        $project: {
            _id: 0,
            driver_id: "$_id",
            driver_name: { $concat: ["$driver_info.first_name", " ", "$driver_info.last_name"] },
            car_model: "$driver_info.car_model",
            car_number: "$driver_info.car_number",
            rating: "$driver_info.rating",
            total_rides: 1,
            total_earnings: { $round: ["$total_earnings", 2] },
            avg_price: { $round: ["$avg_price", 2] }
        }
    },
    
    // Стадия 6: Сортировка по earnings
    {
        $sort: { total_earnings: -1 }
    }
]).toArray();

printjson(driverStats);

// АГРЕГАЦИЯ 3: Ежедневная статистика поездок
print("\n3. DAILY RIDE STATISTICS AGGREGATION:");

var dailyStats = db.rides.aggregate([
    // Стадия 1: Проекция - извлекаем дату из created_at
    {
        $project: {
            date: {
                $dateToString: {
                    format: "%Y-%m-%d",
                    date: "$timestamps.created_at"
                }
            },
            status: 1,
            price: 1,
            user_id: 1,
            driver_id: 1
        }
    },
    
    // Стадия 2: Группировка по дате и статусу
    {
        $group: {
            _id: {
                date: "$date",
                status: "$status"
            },
            count: { $sum: 1 },
            total_revenue: { $sum: "$price" },
            unique_users: { $addToSet: "$user_id" },
            unique_drivers: { $addToSet: "$driver_id" }
        }
    },
    
    // Стадия 3: Проекция с подсчетом уникальных
    {
        $project: {
            date: "$_id.date",
            status: "$_id.status",
            count: 1,
            total_revenue: { $round: ["$total_revenue", 2] },
            unique_users_count: { $size: "$unique_users" },
            unique_drivers_count: { $size: "$unique_drivers" }
        }
    },
    
    // Стадия 4: Сортировка по дате
    {
        $sort: { date: -1, status: 1 }
    }
]).toArray();

printjson(dailyStats);

// АГРЕГАЦИЯ 4: Поиск ближайших свободных водителей
print("\n4. FIND NEARBY FREE DRIVERS AGGREGATION:");

// Координаты точки поиска (Москва, ул. Ленина)
var searchPoint = {
    type: "Point",
    coordinates: [37.6176, 55.7558]
};

var nearbyDrivers = db.drivers.aggregate([
    // Стадия 1: Фильтрация - только свободные водители
    {
        $match: {
            status: "free",
            location: { $exists: true }
        }
    },
    
    // Стадия 2: Геопространственный поиск
    {
        $geoNear: {
            near: searchPoint,
            distanceField: "distance_meters",
            spherical: true,
            maxDistance: 5000,  // 5 км
            distanceMultiplier: 1
        }
    },
    
    // Стадия 3: Сортировка по расстоянию
    {
        $sort: { distance_meters: 1 }
    },
    
    // Стадия 4: Ограничение (топ-10)
    {
        $limit: 10
    },
    
    // Стадия 5: Проекция
    {
        $project: {
            _id: 0,
            driver_id: "$_id",
            driver_name: { $concat: ["$first_name", " ", "$last_name"] },
            car_model: 1,
            car_number: 1,
            rating: 1,
            distance_km: { $round: [{ $divide: ["$distance_meters", 1000] }, 2] },
            location: 1
        }
    }
]).toArray();

printjson(nearbyDrivers);

// АГРЕГАЦИЯ 5: Тенденции по времени суток
print("\n5. RIDE TRENDS BY HOUR OF DAY:");

var hourlyTrends = db.rides.aggregate([
    // Стадия 1: Фильтрация - только завершенные поездки с временем
    {
        $match: {
            status: "completed",
            "timestamps.created_at": { $exists: true }
        }
    },
    
    // Стадия 2: Проекция - извлекаем час
    {
        $project: {
            hour: { $hour: "$timestamps.created_at" },
            price: 1,
            day_of_week: { $dayOfWeek: "$timestamps.created_at" }
        }
    },
    
    // Стадия 3: Группировка по часу
    {
        $group: {
            _id: "$hour",
            total_rides: { $sum: 1 },
            avg_price: { $avg: "$price" },
            total_revenue: { $sum: "$price" }
        }
    },
    
    // Стадия 4: Сортировка по часу
    {
        $sort: { _id: 1 }
    },
    
    // Стадия 5: Проекция
    {
        $project: {
            _id: 0,
            hour: "$_id",
            total_rides: 1,
            avg_price: { $round: ["$avg_price", 2] },
            total_revenue: { $round: ["$total_revenue", 2] }
        }
    }
]).toArray();

printjson(hourlyTrends);

// АГРЕГАЦИЯ 6: Рейтинг популярных направлений
print("\n6. POPULAR ROUTES RANKING:");

var popularRoutes = db.rides.aggregate([
    // Стадия 1: Фильтрация - только завершенные поездки
    {
        $match: {
            status: "completed",
            start_address: { $exists: true },
            end_address: { $exists: true }
        }
    },
    
    // Стадия 2: Группировка по маршруту
    {
        $group: {
            _id: {
                start: "$start_address",
                end: "$end_address"
            },
            total_rides: { $sum: 1 },
            avg_price: { $avg: "$price" },
            total_revenue: { $sum: "$price" }
        }
    },
    
    // Стадия 3: Фильтрация - только маршруты с количеством > 1
    {
        $match: {
            total_rides: { $gt: 1 }
        }
    },
    
    // Стадия 4: Сортировка по популярности
    {
        $sort: { total_rides: -1 }
    },
    
    // Стадия 5: Ограничение (топ-10)
    {
        $limit: 10
    },
    
    // Стадия 6: Проекция
    {
        $project: {
            _id: 0,
            start_address: "$_id.start",
            end_address: "$_id.end",
            total_rides: 1,
            avg_price: { $round: ["$avg_price", 2] },
            total_revenue: { $round: ["$total_revenue", 2] }
        }
    }
]).toArray();

printjson(popularRoutes);

// АГРЕГАЦИЯ 7: Анализ отмен поездок
print("\n7. RIDE CANCELLATION ANALYSIS:");

var cancellationAnalysis = db.rides.aggregate([
    // Стадия 1: Группировка по статусу
    {
        $group: {
            _id: "$status",
            count: { $sum: 1 },
            avg_cancellation_time: {
                $avg: {
                    $cond: [
                        { $eq: ["$status", "cancelled"] },
                        { $subtract: ["$timestamps.cancelled_at", "$timestamps.created_at"] },
                        null
                    ]
                }
            }
        }
    },
    
    // Стадия 2: Проекция
    {
        $project: {
            status: "$_id",
            count: 1,
            percentage: {
                $multiply: [
                    { $divide: ["$count", { $sum: "$count" }] },
                    100
                ]
            },
            avg_cancellation_time_seconds: {
                $round: [{ $divide: ["$avg_cancellation_time", 1000] }, 0]
            }
        }
    },
    
    // Стадия 3: Сортировка
    {
        $sort: { count: -1 }
    }
]).toArray();

printjson(cancellationAnalysis);

// АГРЕГАЦИЯ 8: Активные водители в реальном времени
print("\n8. REAL-TIME ACTIVE DRIVERS:");

var activeDrivers = db.drivers.aggregate([
    // Стадия 1: Фильтрация - только занятые водители
    {
        $match: {
            status: "busy",
            current_ride: { $exists: true }
        }
    },
    
    // Стадия 2: JOIN с rides
    {
        $lookup: {
            from: "rides",
            localField: "current_ride.ride_id",
            foreignField: "_id",
            as: "ride_info"
        }
    },
    
    // Стадия 3: Разворачивание
    {
        $unwind: "$ride_info"
    },
    
    // Стадия 4: Проекция
    {
        $project: {
            _id: 0,
            driver_id: "$_id",
            driver_name: { $concat: ["$first_name", " ", "$last_name"] },
            car_model: 1,
            car_number: 1,
            rating: 1,
            current_ride: {
                ride_id: "$current_ride.ride_id",
                start_address: "$current_ride.start_address",
                end_address: "$current_ride.end_address",
                accepted_at: "$current_ride.accepted_at",
                ride_status: "$ride_info.status"
            }
        }
    }
]).toArray();

printjson(activeDrivers);

// АГРЕГАЦИЯ 9: Анализ лояльности пользователей
print("\n9. USER LOYALTY ANALYSIS:");

var userLoyalty = db.rides.aggregate([
    // Стадия 1: Группировка по пользователю
    {
        $group: {
            _id: "$user_id",
            total_rides: { $sum: 1 },
            total_spent: { $sum: "$price" },
            last_ride_date: { $max: "$timestamps.created_at" },
            first_ride_date: { $min: "$timestamps.created_at" }
        }
    },
    
    // Стадия 2: Проекция для вычисления лояльности
    {
        $project: {
            user_id: "$_id",
            total_rides: 1,
            total_spent: { $round: ["$total_spent", 2] },
            avg_rides_per_month: {
                $cond: [
                    { $gt: ["$total_rides", 0] },
                    { $divide: ["$total_rides", 1] }, 
                    0
                ]
            },
            loyalty_tier: {
                $switch: {
                    branches: [
                        { case: { $gte: ["$total_rides", 50] }, then: "Platinum" },
                        { case: { $gte: ["$total_rides", 20] }, then: "Gold" },
                        { case: { $gte: ["$total_rides", 10] }, then: "Silver" },
                        { case: { $gte: ["$total_rides", 5] }, then: "Bronze" }
                    ],
                    default: "New"
                }
            }
        }
    },
    
    // Стадия 3: JOIN с пользователями
    {
        $lookup: {
            from: "users",
            localField: "user_id",
            foreignField: "_id",
            as: "user_info"
        }
    },
    
    // Стадия 4: Разворачивание
    {
        $unwind: "$user_info"
    },
    
    // Стадия 5: Проекция
    {
        $project: {
            _id: 0,
            user_name: { $concat: ["$user_info.first_name", " ", "$user_info.last_name"] },
            user_login: "$user_info.login",
            total_rides: 1,
            total_spent: 1,
            loyalty_tier: 1
        }
    },
    
    // Стадия 6: Сортировка по количеству поездок
    {
        $sort: { total_rides: -1 }
    }
]).toArray();

printjson(userLoyalty);

// АГРЕГАЦИЯ 10: Прогнозирование спроса (по дням недели)
print("\n10. DEMAND FORECASTING BY DAY OF WEEK:");

var demandForecast = db.rides.aggregate([
    // Стадия 1: Проекция с днем недели
    {
        $project: {
            day_of_week: { $dayOfWeek: "$timestamps.created_at" },
            hour: { $hour: "$timestamps.created_at" },
            price: 1,
            status: 1
        }
    },
    
    // Стадия 2: Фильтрация - только завершенные
    {
        $match: {
            status: "completed"
        }
    },
    
    // Стадия 3: Группировка по дню недели и часу
    {
        $group: {
            _id: {
                day: "$day_of_week",
                hour: "$hour"
            },
            avg_rides: { $avg: 1 },
            total_rides: { $sum: 1 },
            avg_price: { $avg: "$price" }
        }
    },
    
    // Стадия 4: Сортировка
    {
        $sort: { "_id.day": 1, "_id.hour": 1 }
    },
    
    // Стадия 5: Проекция
    {
        $project: {
            _id: 0,
            day_of_week: "$_id.day",
            hour: "$_id.hour",
            avg_rides_per_day: { $round: ["$avg_rides", 2] },
            total_rides: 1,
            avg_price: { $round: ["$avg_price", 2] }
        }
    }
]).toArray();

// Преобразование номера дня в название
var dayNames = {
    1: "Sunday",
    2: "Monday",
    3: "Tuesday",
    4: "Wednesday",
    5: "Thursday",
    6: "Friday",
    7: "Saturday"
};

print("Demand forecast by day of week and hour:");
demandForecast.forEach(function(item) {
    print(dayNames[item.day_of_week] + " " + item.hour + ":00 - " +
          item.avg_rides_per_day + " avg rides, $" + item.avg_price);
});

print("\n=== ALL AGGREGATIONS COMPLETED ===");