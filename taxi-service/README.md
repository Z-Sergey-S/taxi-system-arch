# Такси API на Yandex Userver

Сервис для заказа такси, написанный на фреймворке [Yandex Userver](https://github.com/userver-framework/userver). 

## Что умеет сервис

Сервис предоставляет **11 методов API** для работы с такси:

| № | Метод | Эндпоинт | Описание | Аутентификация |
|---|-------|----------|----------|----------------|
| 1 | `POST` | `/api/v1/auth/register` | Регистрация пользователя | Нет |
| 2 | `POST` | `/api/v1/auth/login` | Вход в систему | Нет |
| 3 | `POST` | `/api/v1/users` | Создание пользователя | Нет |
| 4 | `GET` | `/api/v1/users/{login}` | Поиск пользователя по логину | Нет |
| 5 | `GET` | `/api/v1/users/search?first_name=&last_name=` | Поиск по маске имени/фамилии | Нет |
| 6 | `POST` | `/api/v1/drivers` | Регистрация водителя | Нет |
| 7 | `POST` | `/api/v1/rides` | Создание заказа | **Да** (JWT) |
| 8 | `GET` | `/api/v1/rides/active` | Активные заказы | **Да** (JWT) |
| 9 | `POST` | `/api/v1/rides/{ride_id}/accept` | Принятие заказа водителем | **Да** (JWT) |
| 10 | `GET` | `/api/v1/users/{user_id}/rides` | История поездок пользователя | Нет |
| 11 | `POST` | `/api/v1/rides/{ride_id}/complete` | Завершение поездки | **Да** (JWT) |

### Как это работает внутри

- **База данных PostgreSQL** — все данные хранятся в реляционной БД
- **JWT аутентификация** — упрощенная реализация для демонстрации
- **Потокобезопасность** — пул соединений PostgreSQL обеспечивает безопасность
- **Уникальные ID** — UUID генерируются на стороне БД


---

## База данных (PostgreSQL)

### Схема БД

Проект использует PostgreSQL для хранения данных. Схема состоит из трех основных таблиц:

```sql
-- Пользователи
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    login VARCHAR(50) NOT NULL UNIQUE,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Водители
CREATE TABLE drivers (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    car_model VARCHAR(200) NOT NULL,
    car_number VARCHAR(20) NOT NULL UNIQUE,
    status VARCHAR(20) DEFAULT 'free',
    rating DECIMAL(3,2) DEFAULT 5.0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Поездки
CREATE TABLE rides (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    driver_id UUID REFERENCES drivers(id) ON DELETE SET NULL,
    start_address TEXT NOT NULL,
    end_address TEXT NOT NULL,
    status VARCHAR(20) DEFAULT 'created',
    price DECIMAL(10,2) DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    accepted_at TIMESTAMP WITH TIME ZONE,
    completed_at TIMESTAMP WITH TIME ZONE
);
```

### Индексы для оптимизации

Для ускорения запросов созданы следующие индексы:

| Таблица | Индекс | Назначение |
|---------|--------|-------------|
| users | idx_users_login | Быстрый поиск по логину |
| users | idx_users_name | Поиск по маске имени/фамилии |
| drivers | idx_drivers_status | Поиск свободных водителей |
| rides | idx_rides_user_id | JOIN с таблицей users |
| rides | idx_rides_status | Фильтрация по статусу |
| rides | idx_rides_created_at | Сортировка по дате |

### Тестовые данные

В БД предварительно загружены тестовые данные:
- 10 пользователей
- 10 водителей
- 10 поездок

---

## Требования

- **Docker** и **Docker Compose** (рекомендуемый способ)
- Или **C++20 компилятор** (Clang 18+), **CMake 3.12+**, **make**
- **PostgreSQL 14+** (автоматически поднимается в Docker)

---

## Как запустить

### Вариант 1: Через Docker (рекомендуемый)

```bash
# Клонируем репозиторий
git clone https://github.com/Z-Sergey-S/taxi-system-arch.git
cd taxi-system-arch/taxi-service

# Собираем образ
docker-compose build --no-cache

# Запускаем сервис
docker-compose up -d

# Проверяем, что работает
curl http://localhost:8080/ping
# Должен вернуть пустой ответ с кодом 200

# Смотрим логи
docker-compose logs -f taxi-api
```

**Остановить сервис:** `docker-compose down`

---

### Вариант 2: В режиме разработки (через VS Code + Dev Container)

```bash
# Собираем проект
make build-debug

# Запускаем сервис
make start-debug
```

Проверяем:

```bash
curl http://localhost:8080/ping
# Должен вернуть пустой ответ с кодом 200
```

---

## Как работать с аутентификацией

### Шаг 1: Регистрация

```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secret123",
    "first_name": "John",
    "last_name": "Doe",
    "email": "john@example.com"
  }'
```

### Шаг 2: Вход и получение токена

```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "john_doe",
    "password": "secret123"
  }'
```

Пример ответа:
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.xxxxx",
  "user_id": "3a572ffbe81345ec97cbc67cccb6a054",
  "login": "john_doe"
}
```

### Шаг 3: Используем токен в запросах

```bash
TOKEN="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.xxxxx"

# Создаём заказ (требует аутентификации)
curl -X POST http://localhost:8080/api/v1/rides \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "user_id": "3a572ffbe81345ec97cbc67cccb6a054",
    "start_address": "ул. Ленина, 10",
    "end_address": "ул. Пушкина, 20"
  }'

# Получаем активные заказы (требует аутентификации)
curl -X GET http://localhost:8080/api/v1/rides/active \
  -H "Authorization: Bearer $TOKEN"
```

---

## Примеры запросов

### Создание пользователя (без токена)

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

### Поиск пользователя по логину

```bash
curl -X GET "http://localhost:8080/api/v1/users/john_doe"
```

### Поиск пользователей по маске имени и фамилии

```bash
curl -X GET "http://localhost:8080/api/v1/users/search?first_name=Jo&last_name=Do"
```

### Регистрация водителя

```bash
curl -X POST http://localhost:8080/api/v1/drivers \
  -H "Content-Type: application/json" \
  -d '{
    "first_name": "Иван",
    "last_name": "Петров",
    "car_model": "Toyota Camry",
    "car_number": "A123BC"
  }'
```

### Получение истории поездок пользователя

```bash
curl -X GET "http://localhost:8080/api/v1/users/3a572ffbe81345ec97cbc67cccb6a054/rides"
```

---

## Swagger UI

Интерактивная документация API доступна по адресу: **http://localhost:8080/docs**

Swagger UI позволяет:
- Просматривать все 11 endpoints
- Тестировать запросы прямо из браузера
- Добавлять JWT токен для авторизации (кнопка "Authorize")

---

## Структура проекта

```
taxi-service/
├── configs/                    # Конфигурация сервиса
│   ├── static_config.yaml      # Основной конфиг
│   └── config_vars.yaml        # Переменные окружения
├── sql/                        # SQL скрипты для PostgreSQL
│   ├── schema.sql              # Схема БД (таблицы, индексы)
│   ├── data.sql                # Тестовые данные
│   ├── queries.sql             # SQL запросы для API
│   └── optimization.md         # Анализ оптимизации (EXPLAIN)
├── src/
│   ├── auth/                   # JWT-аутентификация
│   │   ├── jwt_manager.cpp/hpp
│   │   └── auth_middleware.cpp/hpp
│   ├── db/                     # Репозитории для работы с БД
│   │   ├── user_repository.cpp/hpp
│   │   ├── driver_repository.cpp/hpp
│   │   └── ride_repository.cpp/hpp
│   ├── handlers/               # 11 HTTP хендлеров
│   │   ├── register_user.cpp/hpp
│   │   ├── find_user_by_login.cpp/hpp
│   │   ├── search_users_by_name.cpp/hpp
│   │   ├── register_driver.cpp/hpp
│   │   ├── create_ride.cpp/hpp
│   │   ├── get_active_rides.cpp/hpp
│   │   ├── accept_ride.cpp/hpp
│   │   ├── get_user_rides.cpp/hpp
│   │   ├── complete_ride.cpp/hpp
│   │   ├── auth_login.cpp/hpp
│   │   ├── auth_register.cpp/hpp
│   │   ├── openapi_handler.cpp/hpp
│   │   └── swagger_ui_handler.cpp/hpp
│   ├── models/                 # DTO: User, Driver, Ride
│   │   ├── user.cpp/hpp
│   │   ├── driver.cpp/hpp
│   │   └── ride.cpp/hpp
│   └── main.cpp                # Точка входа
├── tests/                      # Функциональные тесты
├── Dockerfile                  # Продакшн-образ
├── docker-compose.yaml         # Docker Compose
├── openapi.yaml                # OpenAPI 3.0 спецификация
├── CMakeLists.txt              # CMake конфигурация
├── Makefile                    # Make команды
└── README.md                   # Документация
```

---

## Технологии

- **Язык**: C++20
- **Фреймворк**: Yandex Userver
- **База данных**: PostgreSQL 14
- **Аутентификация**: JWT (упрощенная реализация)
- **Документация**: OpenAPI 3.0 + Swagger UI
- **Контейнеризация**: Docker + Docker Compose

---