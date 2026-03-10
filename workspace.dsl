workspace {
    name "Taxi Ordering System"
    description "Система заказа такси"

    model {

        # Акторы
        passenger = person "Пассажир"  "Заказывает поездки, просматривает историю" 
        driver = person "Водитель" "Принимает поездки, выполняет заказы"
        

        # Внешние системы
        paymentGateway = softwareSystem "Payment Gateway" "Сервис обработки платежей" "external"
        smsService = softwareSystem "SMS Service" "Сервис отправки смс-уведомлений" "external"
        mapService = softwareSystem "Map Service" "Сервис карт и геолокации" "external" 

        # Внутренние системы
        taxiSystem = softwareSystem "Taxi System" {
            description "Система управления заказами такси"
            passenger -> this "Заказ такси"
            driver -> this "Прием заказа"
            this -> paymentGateway "Обработка оплаты" "HTTPS/REST"
            this -> smsService "Отправка уведомлений" "HTTPS/REST"
            this -> mapService "Получение координат и маршрутов" "HTTPS/REST"


            # КОНТЕЙНЕРЫ
            # Приложения
            passengerApp = container "Passenger Mobile App" "Мобильное приложение для пассажиров" "React Native" "mobile"
            driverApp = container "Driver Mobile App" "Мобильное приложение для водителей" "React Native" "mobile"
           
            # api
            apiGateway = container "API Gateway" "Шлюз для маршрутизации запросов" "Node.js Express"
            
            # Сервисы
            passengerService = container "Passenger Service" "Сервис управления пассажирами" "Python, FastAPI"
            driverService = container "Driver Service" "Сервис управления водителями" "Python, FastAPI"
            rideService = container "Ride Service" "Сервис управления поездками" "Python, FastAPI"
            notificationService = container "Notification Service" "Сервис отправки уведомлений" "Python, Celery"
           
            # Базы данных
            dbPassengers = container "Passengers Database" "Хранение данных пассажиров: логин, имя, фамилия, рейтинг, контактная информация, история поездок" "PostgreSQL" "database"
            dbDrivers = container "Drivers Database" "Хранение данных водителей: имя, фамилия, информация об автомобиле, статус, рейтинг, геолокация" "PostgreSQL" "database"
            dbRides = container "Rides Database" "Хранение данных поездок id, пассажир, водитель,атус, начальная/конечная точки, время, стоимость, маршрут" "PostgreSQL" "database"

            # Redis          
            redisCache = container "Redis Cache" "Кэширование активных заказов и сессий" "Redis"

            # Связи между контейнерами
            passengerApp -> apiGateway "Запрос к API" "HTTPS/REST"
            driverApp -> apiGateway "Запрос к API" "HTTPS/REST"

            apiGateway -> passengerService "Маршрутизация /passengers/*" "HTTPS/REST"
            apiGateway -> driverService "Маршрутизация /drivers/*" "HTTPS/REST"
            apiGateway -> rideService "Маршрутизация /rides/*" "HTTPS/REST"

            passengerService -> dbPassengers "Хранение данных" "PostgreSQL"
            passengerService -> dbRides "Получение истории поездок (id, дата, маршрут, стоимость)" "PostgreSQL"
            driverService -> dbDrivers "Хранение данных" "PostgreSQL"
            rideService -> dbRides "Сохранение данных поездки (id, пассажир, водитель, статус, маршрут)" "PostgreSQL"

            rideService -> redisCache "Кэширование активных заказов" "Redis Protocol"
            driverService -> redisCache "Хранение статуса водителей" "Redis Protocol"

            rideService -> notificationService "Триггер уведомлений" "AMQP/RabbitMQ"
            driverService -> notificationService "Уведомление о новых заказах" "AMQP"
            notificationService -> smsService "Отправка SMS" "HTTPS/REST"

            rideService -> paymentGateway "Запрос оплаты" "HTTPS/REST"
            rideService -> mapService "Расчет маршрута и стоимости" "HTTPS/REST"

            # Связи акторов с контейнерами
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

        dynamic taxiSystem "create_and_accept_ride" {
            title "Создание заказа и его принятие водителем"
            
            passenger -> passengerApp "Создает заказ"
            passengerApp -> apiGateway "POST /rides"
            apiGateway -> rideService "Создает поездку"
            rideService -> dbRides "Сохраняет в БД"
            rideService -> redisCache "Добавляет в активные"

            driver -> driverApp "Получает список активных заказов"
            driverApp -> apiGateway "GET /rides/active"
            apiGateway -> rideService "Возвращает заказы"
            rideService -> redisCache "Получает из кэша"

            driver -> driverApp "Принимает заказ"
            driverApp -> apiGateway "PUT /rides/{id}/accept"
            apiGateway -> rideService "Обновляет статус"
            rideService -> dbRides "Сохраняет изменения"

            autoLayout lr
        
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