-- Схема базы данных для Такси-сервиса
-- Сущности: Пользователь, Водитель, Поездка

-- Удаляем старую схему если существует
DROP SCHEMA IF EXISTS taxi_schema CASCADE;

-- Создаем новую схему
CREATE SCHEMA IF NOT EXISTS taxi_schema;

-- Устанавливаем схему по умолчанию
SET search_path TO taxi_schema;


-- ТАБЛИЦА: users
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    login VARCHAR(50) NOT NULL UNIQUE,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    
    -- Ограничения
    CONSTRAINT users_login_check CHECK (length(login) >= 3),
    CONSTRAINT users_email_check CHECK (email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$')
);


-- ТАБЛИЦА: drivers
CREATE TABLE IF NOT EXISTS drivers (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    car_model VARCHAR(200) NOT NULL,
    car_number VARCHAR(20) NOT NULL UNIQUE,
    status VARCHAR(20) NOT NULL DEFAULT 'free',
    rating DECIMAL(3,2) DEFAULT 5.0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    
    -- Ограничения
    CONSTRAINT drivers_status_check CHECK (status IN ('free', 'busy', 'offline')),
    CONSTRAINT drivers_rating_check CHECK (rating >= 0 AND rating <= 5)
);


-- ТАБЛИЦА: rides (Поездки)
CREATE TABLE IF NOT EXISTS rides (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL,
    driver_id UUID,
    start_address TEXT NOT NULL,
    end_address TEXT NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'created',
    price DECIMAL(10,2) DEFAULT 0,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    accepted_at TIMESTAMP WITH TIME ZONE,
    completed_at TIMESTAMP WITH TIME ZONE,
    
    -- Внешние ключи
    CONSTRAINT fk_rides_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_rides_driver FOREIGN KEY (driver_id) REFERENCES drivers(id) ON DELETE SET NULL,
    
    -- Ограничения
    CONSTRAINT rides_status_check CHECK (status IN ('created', 'accepted', 'in_progress', 'completed', 'cancelled')),
    CONSTRAINT rides_price_check CHECK (price >= 0)
);


-- ИНДЕКСЫ ДЛЯ ОПТИМИЗАЦИИ ЗАПРОСОВ

-- Индексы для таблицы users
CREATE INDEX idx_users_login ON users(login);                    
CREATE INDEX idx_users_name ON users(first_name, last_name);     
CREATE INDEX idx_users_email ON users(email);                    

-- Индексы для таблицы drivers
CREATE INDEX idx_drivers_status ON drivers(status);             
CREATE INDEX idx_drivers_car_number ON drivers(car_number);      
-- Индексы для таблицы rides
CREATE INDEX idx_rides_user_id ON rides(user_id);                
CREATE INDEX idx_rides_driver_id ON rides(driver_id);            
CREATE INDEX idx_rides_status ON rides(status);                  
CREATE INDEX idx_rides_created_at ON rides(created_at DESC);     
CREATE INDEX idx_rides_user_status ON rides(user_id, status);    
CREATE INDEX idx_rides_driver_status ON rides(driver_id, status); 

-- КОММЕНТАРИИ К ТАБЛИЦАМ И КОЛОНКАМ

COMMENT ON TABLE users IS 'Зарегистрированные пользователи системы';
COMMENT ON COLUMN users.id IS 'Уникальный идентификатор пользователя (UUID)';
COMMENT ON COLUMN users.login IS 'Логин пользователя (уникальный)';
COMMENT ON COLUMN users.password_hash IS 'Хеш пароля пользователя';

COMMENT ON TABLE drivers IS 'Зарегистрированные водители';
COMMENT ON COLUMN drivers.status IS 'Статус водителя: free, busy, offline';
COMMENT ON COLUMN drivers.rating IS 'Рейтинг водителя от 0 до 5';

COMMENT ON TABLE rides IS 'Поездки пользователей';
COMMENT ON COLUMN rides.status IS 'Статус поездки: created, accepted, in_progress, completed, cancelled';
COMMENT ON COLUMN rides.price IS 'Стоимость поездки в рублях';