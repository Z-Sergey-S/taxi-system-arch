## 1. Анализ событий в системе

### 1.1 События (Events)

| Событие | Триггер | Payload |
|---------|---------|---------|
| UserCreated | Пользователь зарегистрировался | user_id, login, name, email |
| RideCreated | Пассажир создал заказ | ride_id, user_id, start_address, end_address, price |
| RideAccepted | Водитель принял заказ | ride_id, driver_id, driver_name, car_number |
| RideCompleted | Поездка завершена | ride_id, final_price, completion_time |

### 1.2 Команды (Commands)

| Команда | Источник | Результат |
|---------|----------|-----------|
| CreateUser | API /users | UserCreated |
| CreateRide | API /rides | RideCreated |
| AcceptRide | API /rides/{id}/accept | RideAccepted |
| CompleteRide | API /rides/{id}/complete | RideCompleted |

### 1.3 Потребители событий

| Событие | Потребитель | Действие |
|---------|-------------|----------|
| UserCreated | Notification Service | Отправить приветственное письмо |
| RideCreated | Matching Service | Найти ближайших водителей |
| RideAccepted | Notification Service | Уведомить пассажира |
| RideCompleted | Billing Service | Списать средства |

## 2. Event-Driven архитектура

### 2.1 Компоненты

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   API       │────▶│  RabbitMQ   │────▶│  Consumer   │
│  (Producer) │     │  (Broker)   │     │  (Handler)  │
└─────────────┘     └─────────────┘     └─────────────┘
```

### 2.2 Exchange и Routing

- Exchange: `taxi.events` (type: topic)
- Routing keys:
  - `user.created`
  - `ride.created`
  - `ride.accepted`
  - `ride.completed`

## 3. Формат сообщений

```json
{
  "event_id": "uuid",
  "event_type": "RideCreated",
  "timestamp": 1234567890,
  "data": { ... }
}
```

## 4. Гарантии доставки

- At-least-once delivery
- Persistent messages
- Consumer acknowledgements


