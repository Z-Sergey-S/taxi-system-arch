```markdown
# Такси API на Yandex Userver с MongoDB

Сервис для заказа такси, написанный на фреймворке [Yandex Userver](https://github.com/userver-framework/userver) с использованием MongoDB в качестве базы данных.

---

## Домашнее задание №4: Проектирование и работа с MongoDB

**Студент:** Жеребцов Сергей  
**Группа:** М8О-103СВ-25  
**Вариант:** №16 - Система заказа такси (https://www.uber.com/)

---

## Что умеет сервис

Сервис предоставляет **11 методов API** для работы с такси:

| № | Метод | Эндпоинт | Описание | Аутентификация |
|---|-------|----------|----------|----------------|
| 1 | `POST` | `/api/v1/auth/register` | Регистрация пользователя | Нет |
| 2 | `POST` | `/api/v1/auth/login` | Вход в систему | Нет |
| 3 | `POST` | `/api/v1/users` | Создание пользователя | Нет |
| 4 | `GET` | `/api/v1/users/{login}` | Поиск пользователя по логину | Нет |
| 5 | `GET` | `/api/v1/users/search` | Поиск по маске имени/фамилии | Нет |
| 6 | `POST` | `/api/v1/drivers` | Регистрация водителя | Нет |
| 7 | `POST` | `/api/v1/rides` | Создание заказа | **Да** (JWT) |
| 8 | `GET` | `/api/v1/rides/active` | Активные заказы | **Да** (JWT) |
| 9 | `POST` | `/api/v1/rides/{ride_id}/accept` | Принятие заказа водителем | **Да** (JWT) |
| 10 | `GET` | `/api/v1/users/{user_id}/rides` | История поездок пользователя | Нет |
| 11 | `POST` | `/api/v1/rides/{ride_id}/complete` | Завершение поездки | **Да** (JWT) |

---

## Документная модель MongoDB

### Коллекция `users`

```json
{
  "_id": ObjectId("..."),
  "login": "john_doe",
  "first_name": "John",
  "last_name": "Doe",
  "email": "john@example.com",
  "password_hash": "hash_secret123",
  "created_at": ISODate("...")
}
```

### Коллекция `drivers`

```json
{
  "_id": ObjectId("..."),
  "login": "driver_ivan",
  "first_name": "Ivan",
  "last_name": "Petrov",
  "car_model": "Toyota Camry",
  "car_number": "A123BC",
  "password_hash": "hash_driverpass",
  "status": "free",
  "rating": 4.8,
  "created_at": ISODate("...")
}
```

### Коллекция `rides`

```json
{
  "_id": ObjectId("..."),
  "user_id": ObjectId("..."),
  "driver_id": ObjectId("..."),
  "start_address": "ул. Ленина, 10",
  "end_address": "ул. Пушкина, 20",
  "status": "completed",
  "price": 510.00,
  "created_at": ISODate("..."),
  "accepted_at": ISODate("..."),
  "completed_at": ISODate("...")
}
```

### Выбор между Embedded и References

| Связь | Тип | Решение | Обоснование |
|-------|-----|---------|-------------|
| Пользователь → Активная поездка | 1:0..1 | **Embedded** | У пользователя может быть только одна активная поездка, частый доступ |
| Пользователь → История поездок | 1:N | **References** | Поездок может быть неограниченное количество |
| Водитель → Текущая поездка | 1:0..1 | **Embedded** | Водитель может выполнять только одну поездку одновременно |
| Поездка → Временные метки | 1:1 | **Embedded** | Логически связанные данные |

---

## 🔧 Индексы для оптимизации

| Коллекция | Индекс | Назначение |
|-----------|--------|------------|
| users | login (unique) | Быстрый поиск по логину |
| users | email (unique) | Проверка уникальности email |
| drivers | car_number (unique) | Проверка уникальности номера |
| drivers | status | Поиск свободных водителей |
| rides | user_id | JOIN с пользователями |
| rides | driver_id | JOIN с водителями |
| rides | status | Фильтрация активных заказов |
| rides | created_at | Сортировка по дате |

---

## Запуск через Docker

```bash
# Клонируем репозиторий
git clone https://github.com/Z-Sergey-S/taxi-system-arch.git
cd taxi-system-arch/taxi-service-mongodb

# Собираем образ
docker compose build --no-cache

# Запускаем сервис
docker compose up -d

# Проверяем, что работает
curl http://localhost:8080/ping

# Смотрим логи
docker compose logs -f taxi-api

# Останавливаем
docker compose down
```

---

## Тестирование API

### Запуск тестового скрипта

```bash
chmod +x test-api.sh
./test-api.sh
```

### Примеры запросов

#### Создание пользователя

```bash
curl -X POST http://localhost:8080/api/v1/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secret123",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com"
  }'
```

#### Аутентификация

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secret123"
  }'
```

#### Регистрация водителя

```bash
curl -X POST http://localhost:8080/api/v1/drivers \
  -H "Content-Type: application/json" \
  -d '{
    "login": "driver_ivan",
    "first_name": "Ivan",
    "last_name": "Petrov",
    "car_model": "Toyota Camry",
    "car_number": "A123BC",
    "password": "driverpass"
  }'
```

#### Создание заказа (требует JWT токен)

```bash
TOKEN="your-jwt-token"
curl -X POST http://localhost:8080/api/v1/rides \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "user_id": "user-id",
    "start_address": "ул. Ленина, 10",
    "end_address": "ул. Пушкина, 20"
  }'
```

---

## Агрегационные запросы

### Статистика по пользователю

```javascript
db.rides.aggregate([
    { $match: { status: "completed" } },
    { $group: {
        _id: "$user_id",
        total_rides: { $sum: 1 },
        total_spent: { $sum: "$price" }
    }},
    { $sort: { total_spent: -1 } }
])
```

### Топ водителей по заработку

```javascript
db.rides.aggregate([
    { $match: { driver_id: { $exists: true }, status: "completed" } },
    { $group: {
        _id: "$driver_id",
        total_earnings: { $sum: "$price" },
        total_rides: { $sum: 1 }
    }},
    { $sort: { total_earnings: -1 } },
    { $limit: 10 }
])
```

### Поиск ближайших водителей

```javascript
db.drivers.aggregate([
    { $geoNear: {
        near: { type: "Point", coordinates: [37.6176, 55.7558] },
        distanceField: "distance_meters",
        maxDistance: 5000,
        spherical: true
    }},
    { $match: { status: "free" } },
    { $limit: 10 }
])
```

---

## Структура проекта

```
taxi-service-mongodb/
├── src/
│   ├── auth/                 # JWT-аутентификация
│   ├── handlers/             # 11 HTTP хендлеров
│   ├── models/               # DTO: User, Driver, Ride
│   └── main.cpp              # Точка входа
├── configs/
│   └── static_config.yaml    # Конфигурация с MongoDB
├── scripts/
│   ├── mongo-init.js         # Инициализация БД (коллекции, индексы)
│   ├── queries.js            # MongoDB CRUD запросы
│   ├── validation.js         # Валидация схем
│   └── aggregations.js       # Агрегационные запросы
├── docs/
│   └── schema_design.md      # Документация по модели данных
├── Dockerfile
├── docker-compose.yaml
├── openapi.yaml
├── test-api.sh               # Скрипт для тестирования API
└── README.md
```

---

##  Технологии

- **Язык**: C++20
- **Фреймворк**: Yandex Userver
- **База данных**: MongoDB 6.0
- **Аутентификация**: JWT
- **Документация**: OpenAPI 3.0 + Swagger UI
- **Контейнеризация**: Docker + Docker Compose

---


## Выполнение скриптов

```bash
# Инициализация БД
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/mongo-init.js

# CRUD запросы
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/queries.js

# Валидация
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/validation.js

# Агрегации
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/aggregations.js
```

### Swagger UI

Откройте в браузере: **http://localhost:8080/docs**

---


