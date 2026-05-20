# Event Catalog

## UserCreated

| Поле | Тип | Описание |
|------|-----|----------|
| event_id | string | Уникальный ID события |
| event_type | string | "UserCreated" |
| timestamp | int64 | Время события (Unix timestamp) |
| data.user_id | string | ID пользователя |
| data.login | string | Логин пользователя |
| data.first_name | string | Имя |
| data.last_name | string | Фамилия |
| data.email | string | Email |

**Producer:** RegisterUserHandler
**Consumer:** NotificationService (логирование)
**Routing Key:** user.created

## RideCreated

| Поле | Тип | Описание |
|------|-----|----------|
| event_id | string | Уникальный ID события |
| event_type | string | "RideCreated" |
| timestamp | int64 | Время события |
| data.ride_id | string | ID заказа |
| data.user_id | string | ID пользователя |
| data.start_address | string | Адрес отправления |
| data.end_address | string | Адрес назначения |
| data.price | double | Цена |

**Producer:** CreateRideHandler
**Consumer:** MatchingService (логирование)
**Routing Key:** ride.created

## RideAccepted

| Поле | Тип | Описание |
|------|-----|----------|
| event_id | string | Уникальный ID события |
| event_type | string | "RideAccepted" |
| timestamp | int64 | Время события |
| data.ride_id | string | ID заказа |
| data.driver_id | string | ID водителя |
| data.driver_name | string | Имя водителя |
| data.car_number | string | Номер автомобиля |

**Producer:** AcceptRideHandler
**Consumer:** NotificationService (логирование)
**Routing Key:** ride.accepted

## RideCompleted

| Поле | Тип | Описание |
|------|-----|----------|
| event_id | string | Уникальный ID события |
| event_type | string | "RideCompleted" |
| timestamp | int64 | Время события |
| data.ride_id | string | ID заказа |
| data.final_price | double | Финальная цена |
| data.completion_time | string | Время завершения |

**Producer:** CompleteRideHandler
**Consumer:** BillingService (логирование)
**Routing Key:** ride.completed
