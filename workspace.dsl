workspace {
    name "Taxi Ordering System"
    description "Система заказа такси"

    model {

        passenger = person "Пассажир"  "Заказывает поездки, просматривает историю" 
        driver = person "Водитель" "Принимает поездки, выполняет заказы"
        

        # Внешние системы
        paymentGateway = softwareSystem "Payment Gateway" "Сервис обработки платежей" "external"
        smsService = softwareSystem "SMS Service" "Сервис отправки смс-уведомлений" "external"

        # Внутренние системы
        taxiSystem = softwareSystem "Taxi System" {
            description "Система управления заказами такси"
            passenger -> this "Заказ такси"
            driver -> this "Прием заказа"
            this -> paymentGateway "Обработка оплаты" "HTTPS/REST"
            this -> smsService "Отправка уведомлений" "HTTPS/REST"


            # КОНТЕЙНЕРЫ
            # Приложения
            passengerApp = container "Passenger Mobile App" "Мобильное приложение для пассажиров" "React Native" "mobile"
            driverApp = container "Driver Mobile App" "Мобильное приложение для водетелей" "React Native" "mobile"
           
            # api
            apiGateway = container "API Gateway" "Шлюз для маршрутизации запросов" "Node.js Express"
            
            # Сервисы
            passengerService = container "Passenger Service" "Сервис управления пассажирами" "Python, FastAPI"
            driverService = container "Driver Service" "Сервис управления водителями" "Python, FastAPI"
            rideService = container "Ride Service" "Сервис управления поездками" "Python, FastAPI"
            notificationService = container "Notification Service" "Сервис отправки уведомлений" "Python, Celery"
           
            # Базы данных
            dbPassengers = container "Passengers Databasae" "Хранение данных пассажиров" "PostgreSQL" "database"
            dbDrivers = container "Drivers Databasae" "Хранение данных водителей" "PostgreSQL" "database"
            dbRides = container "Rides Databasae" "Хранение данных поездок" "PostgreSQL" "database"

            # Redis          
            redisCache = container "Radis Cache" "Кэширование активных заказов и сессий" "Redis"

            # Связи между контейнерами
            passengerApp -> apiGateway "Запрос к API" "HTTPS/REST"
            driverApp -> apiGateway "Запрос к API" "HTTPS/REST"

            apiGateway -> passengerService "Маршрутизация /passengers/*" "HTTPS/REST"
            apiGateway -> driverService "Маршрутизация /drivers/*" "HTTPS/REST"
            apiGateway -> rideService "Маршрутизация /rides/*" "HTTPS/REST"

            passengerService -> dbPassengers "Хранение данных" "PostgreSQL"
            driverService -> dbDrivers "Хранение данных" "PostgreSQL"
            rideService -> dbRides "Хранение данных" "PostgreSQL"

            rideService -> redisCache "Кэширование активных заказов" "Redis Protocol"
            driverService -> redisCache "Хранение статуса водителей" "Redis Protocol"

            rideService -> notificationService "Триггер уведомлений" "AMQP/RabbitMQ"
            notificationService -> smsService "Отправка SMS" "HTTP/REST"

            rideService -> paymentGateway "Запрос оплаты" "HTTP/REST"

            passenger -> passengerApp "Использует приложение" "Mobile Network"
            driver -> driverApp "Использует приложение" "Mobile Network"

        }        
    }

    views {
        systemContext taxiSystem "system_context" {
            include * 
            autoLayout
        }

        container taxiSystem "containers" {
            include *
            autoLayout
        }

        styles {
            element "external" {
                background #e0e0e0
                color #555555
                shape roundedBox
            }

            relationship "external" {
                color #999999
                dashed true
            }

            element "database" {
                shape Cylinder
            }

            element "mobile" {
                shape MobileDevicePortrait
            }
        }

        themes default

    }

}