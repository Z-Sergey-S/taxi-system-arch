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
            passenger -> this
            driver -> this
            this -> paymentGateway "Обработка оплаты" "HTTPS/REST"
            this -> smsService "Отправка уведомлений" "HTTPS/REST"
        }        
    }

    views {
        systemContext taxiSystem "system_context" {
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
        }

        themes default

    }

}