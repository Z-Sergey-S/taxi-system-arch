# Каталог событий системы заказа такси

## UserCreated

**Routing key:** `user.created`

**Payload:**

```json
{
  "eventId": "550e8400-e29b-41d4-a716-446655440000",
  "eventType": "UserCreated",
  "timestamp": 1678901234567,
  "data": {
    "userId": "60f7b8e5...",
    "login": "john_doe",
    "firstName": "John",
    "lastName": "Doe",
    "email": "john@example.com"
  }
}
```

- **Производитель:** API сервис (`taxi-api`)
- **Потребители:** Нотификации, Аналитика
- **Гарантии доставки:** at-least-once

---

## DriverRegistered

**Routing key:** `driver.registered`

**Payload:**

```json
{
  "eventId": "...",
  "eventType": "DriverRegistered",
  "timestamp": ...,
  "data": {
    "driverId": "...",
    "login": "driver_ivan",
    "carModel": "Toyota Camry",
    "carNumber": "A123BC"
  }
}
```

- **Производитель:** API сервис
- **Потребители:** Нотификации, Аналитика
- **Гарантии доставки:** at-least-once

---

## RideCreated

**Routing key:** `ride.created`

**Payload:**

```json
{
  "eventId": "...",
  "eventType": "RideCreated",
  "timestamp": ...,
  "data": {
    "rideId": "...",
    "userId": "...",
    "startAddress": "ул. Ленина, 10",
    "endAddress": "ул. Пушкина, 20",
    "price": 510.00,
    "createdAt": "2025-03-15T10:30:00Z"
  }
}
```

- **Производитель:** API сервис
- **Потребители:** Нотификации (водителям), Аналитика, Диспетчерская
- **Гарантии доставки:** at-least-once

---

## RideAccepted

**Routing key:** `ride.accepted`

**Payload:**

```json
{
  "eventId": "...",
  "eventType": "RideAccepted",
  "timestamp": ...,
  "data": {
    "rideId": "...",
    "driverId": "...",
    "acceptedAt": "2025-03-15T10:31:00Z"
  }
}
```

- **Производитель:** API сервис
- **Потребители:** Нотификации (пассажиру), Аналитика
- **Гарантии доставки:** at-least-once

---

## RideCompleted

**Routing key:** `ride.completed`

**Payload:**

```json
{
  "eventId": "...",
  "eventType": "RideCompleted",
  "timestamp": ...,
  "data": {
    "rideId": "...",
    "driverId": "...",
    "completedAt": "2025-03-15T11:15:00Z",
    "finalPrice": 510.00
  }
}
```

- **Производитель:** API сервис
- **Потребители:** Биллинг, Аналитика, Нотификации
- **Гарантии доставки:** at-least-once
