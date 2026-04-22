// MongoDB инициализация для системы заказа такси (упрощенная)

db = db.getSiblingDB('taxi_db');

// Удаляем старые коллекции, если есть
db.users.drop();
db.drivers.drop();
db.rides.drop();


// 1. Создание коллекции users (без валидации для начала)
db.createCollection('users');


// 2. Создание коллекции drivers
db.createCollection('drivers');


// 3. Создание коллекции rides
db.createCollection('rides');


// 4. Создание индексов

// Индексы для users
db.users.createIndex({ "login": 1 }, { unique: true });
db.users.createIndex({ "email": 1 }, { unique: true });

// Индексы для drivers
db.drivers.createIndex({ "car_number": 1 }, { unique: true });
db.drivers.createIndex({ "status": 1 });

// Индексы для rides
db.rides.createIndex({ "user_id": 1 });
db.rides.createIndex({ "driver_id": 1 });
db.rides.createIndex({ "status": 1 });
db.rides.createIndex({ "created_at": -1 });


// 5. Вставка тестовых данных

// Пользователи
const users = [
    { login: "john_doe", first_name: "John", last_name: "Doe", email: "john@example.com", password_hash: "hash_secret123", created_at: new Date() },
    { login: "jane_smith", first_name: "Jane", last_name: "Smith", email: "jane@example.com", password_hash: "hash_pass456", created_at: new Date() },
    { login: "bob_wilson", first_name: "Bob", last_name: "Wilson", email: "bob@example.com", password_hash: "hash_bob789", created_at: new Date() },
    { login: "alice_brown", first_name: "Alice", last_name: "Brown", email: "alice@example.com", password_hash: "hash_alice321", created_at: new Date() },
    { login: "charlie_davis", first_name: "Charlie", last_name: "Davis", email: "charlie@example.com", password_hash: "hash_charlie654", created_at: new Date() }
];

for (const user of users) {
    db.users.insertOne(user);
}

// Водители
const drivers = [
    { first_name: "Ivan", last_name: "Petrov", car_model: "Toyota Camry", car_number: "A123BC", status: "free", rating: 4.8, created_at: new Date() },
    { first_name: "Sergey", last_name: "Ivanov", car_model: "Hyundai Solaris", car_number: "B456CD", status: "free", rating: 4.5, created_at: new Date() },
    { first_name: "Alexey", last_name: "Sidorov", car_model: "Kia Rio", car_number: "C789EF", status: "free", rating: 4.9, created_at: new Date() },
    { first_name: "Dmitry", last_name: "Kuznetsov", car_model: "Renault Logan", car_number: "D012GH", status: "free", rating: 4.2, created_at: new Date() },
    { first_name: "Mikhail", last_name: "Volkov", car_model: "Skoda Octavia", car_number: "E345IJ", status: "free", rating: 4.7, created_at: new Date() }
];

for (const driver of drivers) {
    db.drivers.insertOne(driver);
}

// Находим ID пользователей для поездок
const john = db.users.findOne({ login: "john_doe" });
const jane = db.users.findOne({ login: "jane_smith" });
const ivan = db.drivers.findOne({ car_number: "A123BC" });
const sergey = db.drivers.findOne({ car_number: "B456CD" });

// Поездки
if (john && ivan) {
    db.rides.insertOne({
        user_id: john._id,
        driver_id: ivan._id,
        start_address: "ул. Ленина, 10",
        end_address: "ул. Пушкина, 20",
        status: "completed",
        price: 510.00,
        created_at: new Date(),
        accepted_at: new Date(),
        completed_at: new Date()
    });
}

if (jane && sergey) {
    db.rides.insertOne({
        user_id: jane._id,
        driver_id: sergey._id,
        start_address: "пр. Мира, 15",
        end_address: "ул. Гагарина, 25",
        status: "completed",
        price: 320.00,
        created_at: new Date(),
        accepted_at: new Date(),
        completed_at: new Date()
    });
}

// Активная поездка
if (john) {
    db.rides.insertOne({
        user_id: john._id,
        start_address: "ул. Чехова, 5",
        end_address: "ул. Толстого, 8",
        status: "created",
        price: 450.00,
        created_at: new Date()
    });
}

print("=== MongoDB инициализация завершена ===");
print("Users inserted: " + db.users.countDocuments());
print("Drivers inserted: " + db.drivers.countDocuments());
print("Rides inserted: " + db.rides.countDocuments());