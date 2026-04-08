-- =====================================================
-- SQL ЗАПРОСЫ ДЛЯ ВСЕХ ОПЕРАЦИЙ API
-- Такси-сервис
-- =====================================================

SET search_path TO taxi_schema;

-- =====================================================
-- 1. Создание нового пользователя (POST /api/v1/users)
-- =====================================================
-- Демонстрация синтаксиса (без реальной вставки)

SELECT '1. CREATE USER' as test_name;
INSERT INTO users (login, first_name, last_name, email, password_hash)
VALUES ('demo_user', 'Demo', 'User', 'demo@example.com', 'hash_demo123')
ON CONFLICT (login) DO NOTHING
RETURNING id, login, first_name, last_name, email, created_at;

-- =====================================================
-- 2. Поиск пользователя по логину (GET /api/v1/users/{login})
-- =====================================================

SELECT '2. FIND USER BY LOGIN' as test_name;
SELECT id, login, first_name, last_name, email, created_at
FROM users
WHERE login = 'john_doe';

-- =====================================================
-- 3. Поиск пользователя по маске имени и фамилии (GET /api/v1/users/search)
-- =====================================================

SELECT '3. SEARCH USERS BY NAME MASK' as test_name;
SELECT id, login, first_name, last_name, email, created_at
FROM users
WHERE first_name ILIKE '%Jo%' AND last_name ILIKE '%Do%';

-- =====================================================
-- 4. Регистрация водителя (POST /api/v1/drivers)
-- =====================================================

SELECT '4. REGISTER DRIVER' as test_name;
INSERT INTO drivers (first_name, last_name, car_model, car_number)
VALUES ('Demo', 'Driver', 'Demo Car', 'X999XX')
ON CONFLICT (car_number) DO NOTHING
RETURNING id, first_name, last_name, car_model, car_number, status, rating, created_at;

-- =====================================================
-- 5. Создание заказа поездки (POST /api/v1/rides)
-- =====================================================

SELECT '5. CREATE RIDE' as test_name;
INSERT INTO rides (user_id, start_address, end_address)
SELECT id, 'Demo Start', 'Demo End' FROM users WHERE login = 'john_doe' LIMIT 1
RETURNING id, user_id, start_address, end_address, status, price, created_at;

-- =====================================================
-- 6. Получение активных заказов (GET /api/v1/rides/active)
-- =====================================================

SELECT '6. GET ACTIVE RIDES' as test_name;
SELECT r.id, r.user_id, u.login as user_login, r.start_address, r.end_address, 
       r.status, r.price, r.created_at
FROM rides r
JOIN users u ON r.user_id = u.id
WHERE r.status IN ('created', 'accepted')
ORDER BY r.created_at ASC
LIMIT 10;

-- =====================================================
-- 7. Принятие заказа водителем (POST /api/v1/rides/{ride_id}/accept)
-- =====================================================

SELECT '7. ACCEPT RIDE' as test_name;
-- Находим свободного водителя и активную поездку
WITH available_driver AS (
    SELECT id FROM drivers WHERE status = 'free' LIMIT 1
),
pending_ride AS (
    SELECT id FROM rides WHERE status = 'created' LIMIT 1
)
UPDATE rides 
SET driver_id = (SELECT id FROM available_driver),
    status = 'accepted',
    accepted_at = CURRENT_TIMESTAMP
WHERE id = (SELECT id FROM pending_ride)
  AND status = 'created'
RETURNING id, driver_id, status, accepted_at;

-- =====================================================
-- 8. Получение истории поездок пользователя (GET /api/v1/users/{user_id}/rides)
-- =====================================================

SELECT '8. GET USER RIDE HISTORY' as test_name;
SELECT r.id, r.user_id, u.login as user_login,
       r.driver_id, d.first_name as driver_first_name, d.last_name as driver_last_name,
       r.start_address, r.end_address, r.status, r.price, 
       r.created_at, r.accepted_at, r.completed_at
FROM rides r
LEFT JOIN users u ON r.user_id = u.id
LEFT JOIN drivers d ON r.driver_id = d.id
WHERE r.user_id = (SELECT id FROM users WHERE login = 'john_doe')
ORDER BY r.created_at DESC
LIMIT 10;

-- =====================================================
-- 9. Завершение поездки (POST /api/v1/rides/{ride_id}/complete)
-- =====================================================

SELECT '9. COMPLETE RIDE' as test_name;
-- Завершаем принятую поездку
WITH accepted_ride AS (
    SELECT id, driver_id FROM rides WHERE status = 'accepted' LIMIT 1
)
UPDATE rides 
SET status = 'completed',
    completed_at = CURRENT_TIMESTAMP
WHERE id = (SELECT id FROM accepted_ride)
  AND status = 'accepted'
RETURNING id, status, completed_at;

-- =====================================================
-- 10. Аутентификация пользователя (POST /api/v1/auth/login)
-- =====================================================

SELECT '10. AUTHENTICATE USER' as test_name;
SELECT id, login, first_name, last_name, email, password_hash, created_at
FROM users
WHERE login = 'john_doe';

-- =====================================================
-- 11. Регистрация через auth (POST /api/v1/auth/register)
-- =====================================================

SELECT '11. REGISTER VIA AUTH' as test_name;
INSERT INTO users (login, first_name, last_name, email, password_hash)
VALUES ('auth_demo', 'Auth', 'Demo', 'auth@demo.com', 'hash_auth')
ON CONFLICT (login) DO NOTHING
RETURNING id, login, first_name, last_name, email, created_at;

-- =====================================================
-- ДОПОЛНИТЕЛЬНЫЕ ЗАПРОСЫ ДЛЯ АНАЛИЗА
-- =====================================================

SELECT 'EXTRA: FIND FREE DRIVERS' as test_name;
SELECT id, first_name, last_name, car_model, car_number, rating
FROM drivers
WHERE status = 'free'
ORDER BY rating DESC
LIMIT 5;

SELECT 'EXTRA: USER STATISTICS' as test_name;
SELECT 
    COUNT(*) as total_rides,
    SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed_rides,
    ROUND(AVG(CASE WHEN status = 'completed' THEN price END), 2) as avg_price,
    SUM(CASE WHEN status = 'completed' THEN price ELSE 0 END) as total_spent
FROM rides
WHERE user_id = (SELECT id FROM users WHERE login = 'john_doe');