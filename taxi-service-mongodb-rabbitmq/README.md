# Такси API на Yandex Userver с MongoDB

Сервис для заказа такси, написанный на фреймворке Yandex Userver с использованием MongoDB в качестве базы данных.

---

## Домашние задания

- ДЗ N4: Проектирование и работа с MongoDB
- ДЗ N5: Оптимизация производительности (кеширование + rate limiting)
- ДЗ N6: Event-Driven архитектура (события + RabbitMQ)

**Студент:** Жеребцов Сергей  
**Группа:** М8О-103СВ-25  
**Вариант:** N16 - Система заказа такси (https://www.uber.com/)

---

## Что умеет сервис

Сервис предоставляет 11 методов API для работы с такси:

| N | Метод | Эндпоинт | Описание | Кеш | Rate Limit | Аутентификация |
|---|-------|----------|----------|-----|------------|----------------|
| 1 | POST | /api/v1/auth/register | Регистрация пользователя | Нет | Нет | Нет |
| 2 | POST | /api/v1/auth/login | Вход в систему | Нет | Нет | Нет |
| 3 | POST | /api/v1/users | Создание пользователя | Нет | Нет | Нет |
| 4 | GET | /api/v1/users/{login} | Поиск пользователя по логину | Да (300с) | Нет | Нет |
| 5 | GET | /api/v1/users/search | Поиск по маске имени/фамилии | Да (300с) | Да (10/мин) | Нет |
| 6 | POST | /api/v1/drivers | Регистрация водителя | Нет | Нет | Нет |
| 7 | POST | /api/v1/rides | Создание заказа | Нет | Нет | Да (JWT) |
| 8 | GET | /api/v1/rides/active | Активные заказы | Да (10с) | Да (10/мин) | Да (JWT) |
| 9 | POST | /api/v1/rides/{ride_id}/accept | Принятие заказа водителем | Нет | Нет | Да (JWT) |
| 10 | GET | /api/v1/users/{user_id}/rides | История поездок пользователя | Нет | Нет | Нет |
| 11 | POST | /api/v1/rides/{ride_id}/complete | Завершение поездки | Нет | Нет | Да (JWT) |

---

## Event-Driven архитектура (ДЗ №6)

### События в системе

При выполнении операций публикуются следующие события:

| Событие | Когда происходит | Routing Key |
|---------|-----------------|-------------|
| UserCreated | Регистрация пользователя | user.created |
| RideCreated | Создание заказа | ride.created |
| RideAccepted | Принятие заказа водителем | ride.accepted |
| RideCompleted | Завершение поездки | ride.completed |

### Формат события

```json
{
  "event_id": "550e8400-e29b-41d4-a716-446655440000",
  "event_type": "UserCreated",
  "timestamp": 1703000000,
  "data": {
    "user_id": "123",
    "login": "john_doe",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com"
  }
}
```

### Просмотр событий в логах

```bash
docker compose logs taxi-api | grep "Publishing event"
```

### Пример вывода

```
Publishing event: user.created, event_id: 72e33b27-42fe-4cd8-ab12-cbb64c7ced2a, message: {"event_id":"...","event_type":"UserCreated",...}
```

### RabbitMQ Management UI

Откройте в браузере: http://localhost:15672 (логин: guest, пароль: guest)

---

## Оптимизация производительности (ДЗ №5)

### 1. Кеширование с Redis

Используется стратегия Cache-Aside. При GET запросе сначала проверяется наличие данных в Redis. Если данные найдены, они возвращаются клиенту. Если нет, выполняется запрос к MongoDB, результат сохраняется в кеш и возвращается пользователю.

Кешируемые данные и их TTL:

| Endpoint | Ключ кеша | TTL |
|----------|-----------|-----|
| GET /users/search | user:search:{first_name}:{last_name} | 300 секунд |
| GET /rides/active | rides:active | 10 секунд |

Инвалидация кеша происходит при создании, принятии или завершении заказа - в этих случаях удаляется ключ rides:active.

### 2. Rate Limiting (Token Bucket)

Ограничения:
- GET /api/v1/users/search: 10 запросов в минуту
- GET /api/v1/rides/active: 10 запросов в минуту

При превышении лимита возвращается HTTP 429 Too Many Requests.

---

## Документная модель MongoDB (ДЗ №4)

### Коллекция users
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

### Коллекция drivers
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

### Коллекция rides
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
| Пользователь -> Активная поездка | 1:0..1 | Embedded | У пользователя может быть только одна активная поездка |
| Пользователь -> История поездок | 1:N | References | Поездок может быть неограниченное количество |
| Водитель -> Текущая поездка | 1:0..1 | Embedded | Водитель может выполнять только одну поездку одновременно |

---

## Индексы для оптимизации

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
# Клонирование репозитория
git clone https://github.com/Z-Sergey-S/taxi-system-arch.git
cd taxi-system-arch/taxi-service-mongodb

# Сборка и запуск
docker compose build --no-cache
docker compose up -d

# Проверка работоспособности
curl http://localhost:8080/ping

# Проверка Redis
docker exec -it taxi-redis redis-cli PING

# Проверка RabbitMQ
docker exec -it taxi-rabbitmq rabbitmq-diagnostics ping

# Просмотр логов
docker compose logs -f taxi-api

# Остановка сервиса
docker compose down
```

---

## Тестирование API

### Запуск тестовых скриптов

```bash
chmod +x test-api.sh
./test-api.sh

chmod +x demo-rate-limiting.sh
./demo-rate-limiting.sh
```

### Примеры запросов

**Создание пользователя (публикует событие UserCreated):**
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

**Аутентификация:**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secret123"
  }'
```

**Создание заказа (публикует событие RideCreated):**
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

**Принятие заказа (публикует событие RideAccepted):**
```bash
curl -X POST http://localhost:8080/api/v1/rides/{ride_id}/accept \
  -H "Authorization: Bearer $DRIVER_TOKEN"
```

**Завершение поездки (публикует событие RideCompleted):**
```bash
curl -X POST http://localhost:8080/api/v1/rides/{ride_id}/complete \
  -H "Authorization: Bearer $DRIVER_TOKEN"
```

**Просмотр событий в логах:**
```bash
docker compose logs taxi-api | grep "Publishing event"
```

---

## Структура проекта

```
taxi-service-mongodb/
├── src/
│   ├── auth/                 # JWT-аутентификация
│   ├── cache/                # Redis клиент
│   ├── events/               # Event Publisher (HW6)
│   ├── rate_limit/           # Token Bucket (HW5)
│   ├── handlers/             # HTTP хендлеры
│   ├── models/               # DTO: User, Driver, Ride
│   └── main.cpp              # Точка входа
├── configs/
│   ├── static_config.yaml    # Конфигурация сервера
│   └── config_vars.yaml      # Переменные окружения
├── scripts/
│   ├── mongo-init.js         # Инициализация БД
│   ├── queries.js            # CRUD запросы
│   ├── validation.js         # Валидация схем
│   └── aggregations.js       # Агрегации
├── docs/
│   └── schema_design.md      # Документация по модели (HW4)
├── performance_design.md     # Документация по оптимизации (HW5)
├── event_driven_design.md    # Документация по Event-Driven (HW6)
├── event_catalog.md          # Каталог событий (HW6)
├── Dockerfile
├── docker-compose.yaml
├── openapi.yaml
├── test-api.sh
├── demo-rate-limiting.sh
└── README.md
```

---

## Технологии

- **Язык:** C++20
- **Фреймворк:** Yandex Userver
- **База данных:** MongoDB 6.0
- **Кеширование:** Redis 7
- **Брокер сообщений:** RabbitMQ 3.12
- **Rate Limiting:** Token Bucket
- **Аутентификация:** JWT
- **Документация:** OpenAPI 3.0 + Swagger UI
- **Контейнеризация:** Docker + Docker Compose

---

## Выполнение скриптов

**Инициализация MongoDB:**
```bash
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/mongo-init.js
```

**CRUD запросы:**
```bash
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/queries.js
```

**Валидация схем:**
```bash
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/validation.js
```

**Агрегации:**
```bash
docker exec -i taxi-mongodb mongosh -u admin -p secret --authenticationDatabase admin taxi_db < scripts/aggregations.js
```

**Проверка Redis кеша:**
```bash
docker exec -it taxi-redis redis-cli KEYS "*"
```

**Проверка RabbitMQ:**
```bash
# Проверка состояния
docker exec -it taxi-rabbitmq rabbitmq-diagnostics ping

# Просмотр очередей
docker exec -it taxi-rabbitmq rabbitmqctl list_queues

# Открыть Management UI
# http://localhost:15672 (guest/guest)
```

### Swagger UI

Откройте в браузере: **http://localhost:8080/docs**
