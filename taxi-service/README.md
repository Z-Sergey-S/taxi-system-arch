# Такси API на Yandex Userver

Простой и понятный сервис для заказа такси, написанный на фреймворке [Yandex Userver](https://github.com/userver-framework/userver). Это учебный проект — 9 методов из задания курса «Архитектура программных систем».

---

## Что умеет сервис

Сервис предоставляет **9 методов API** для работы с такси:

| № | Метод | Эндпоинт | Описание |
|---|-------|----------|----------|
| 1 | `POST` | `/api/v1/auth/register` | Регистрация пользователя |
| 2 | `POST` | `/api/v1/auth/login` | Вход в систему |
| 3 | `POST` | `/api/v1/users` | Создание пользователя |
| 4 | `GET` | `/api/v1/users?login=...` | Поиск по логину |
| 5 | `GET` | `/api/v1/users?name=...` | Поиск по имени/фамилии |
| 6 | `POST` | `/api/v1/drivers` | Регистрация водителя |
| 7 | `POST` | `/api/v1/rides` | Создание заказа |
| 8 | `GET` | `/api/v1/rides?status=active` | Активные заказы |
| 9 | `POST` | `/api/v1/rides/{id}/accept` | Принятие заказа |
| — | `GET` | `/api/v1/users/{id}/rides` | История поездок |
| — | `POST` | `/api/v1/rides/{id}/complete` | Завершение поездки |


### Как это работает внутри

- **Хранилище в памяти** — данные живут только пока работает сервис (без базы данных)
- **Простая аутентификация** — пароли хешируются через `hash_<пароль>`
- **Потокобезопасность** — `std::shared_mutex` защищает данные при параллельных запросах
- **Уникальные ID** — генерируются через `userver::utils::Uuid4::GenerateString()`

---

## Как запустить

### Вариант 1: Через Docker

```bash
# Клонируем репозиторий
git clone https://github.com/ваш-логин/taxi-system-arch.git
cd taxi-system-arch/taxi-service

# Собираем образ
docker-compose build --no-cache

# Запускаем сервис
docker-compose up -d

# Проверяем, что работает
curl http://localhost:8080/ping
# Должно вернуть: {"status":"ok"}

# Смотрим логи
docker-compose logs -f taxi-api
```

Остановить сервис: `docker-compose down`

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
# {"status":"ok"}
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

# Создаём водителя
curl -X POST http://localhost:8080/api/v1/drivers \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "first_name": "Иван",
    "last_name": "Петров",
    "car_model": "Toyota Camry",
    "car_number": "А123БС78"
  }'
```

---

## Как протестировать

### Через терминал

```bash
# Простая проверка /ping
curl http://localhost:8080/ping

# Регистрация пользователя
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{"login":"test","password":"pass","first_name":"Test","last_name":"User","email":"test@example.com"}'

# Получаем токен
TOKEN=$(curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"test","password":"pass"}' | jq -r '.token')

# Создаём заказ
curl -X POST http://localhost:8080/api/v1/rides \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"user_id":"test_id","start_address":"ул. Ленина, 10","end_address":"ул. Пушкина, 20"}'
```

### Через тесты

```bash
# В Dev Container
make test-debug

# Или через Docker
docker-compose exec taxi-api python -m pytest tests/ -v
```

---

## Swagger UI

Swagger UI доступен по адресу: http://localhost:8080/docs

---

## Документация

- **OpenAPI спецификация** — файл `openapi.yaml` в корне проекта
- **Структура проекта:**
  ```
  taxi-service/
  ├── configs/          # Конфигурация сервиса
  ├── src/
  │   ├── auth/         # JWT-аутентификация
  │   ├── handlers/     # 9 эндпоинтов (5 файлов)
  │   ├── models/       # DTO: User, Driver, Ride
  │   ├── storage/      # In-memory хранилище
  │   └── main.cpp      # Точка входа
  ├── tests/            # Функциональные тесты
  ├── Dockerfile        # Продакшн-образ
  ├── docker-compose.yaml
  ├── openapi.yaml      # Спецификация API
  └── README.md         
  ```
