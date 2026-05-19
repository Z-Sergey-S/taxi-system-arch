#include "openapi_handler.hpp"
#include <userver/formats/json/serialize.hpp>

namespace handlers {

OpenApiHandler::OpenApiHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string OpenApiHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
    
    // Встроенная OpenAPI спецификация в формате JSON
    const std::string openapi_json = R"({
  "openapi": "3.0.0",
  "info": {
    "title": "Taxi Service API",
    "version": "1.0.0",
    "description": "REST API сервис для заказа такси"
  },
  "servers": [
    {
      "url": "http://localhost:8080/api/v1",
      "description": "Development server"
    }
  ],
  "paths": {
    "/users": {
      "post": {
        "summary": "Создание нового пользователя",
        "tags": ["Users"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "type": "object",
                "required": ["login", "password", "first_name", "last_name", "email"],
                "properties": {
                  "login": {"type": "string"},
                  "password": {"type": "string"},
                  "first_name": {"type": "string"},
                  "last_name": {"type": "string"},
                  "email": {"type": "string", "format": "email"}
                }
              }
            }
          }
        },
        "responses": {
          "201": {"description": "Пользователь создан"},
          "400": {"description": "Ошибка валидации"}
        }
      }
    },
    "/users/{login}": {
      "get": {
        "summary": "Поиск пользователя по логину",
        "tags": ["Users"],
        "parameters": [
          {
            "name": "login",
            "in": "path",
            "required": true,
            "schema": {"type": "string"}
          }
        ],
        "responses": {
          "200": {"description": "Пользователь найден"},
          "404": {"description": "Пользователь не найден"}
        }
      }
    },
    "/users/search": {
      "get": {
        "summary": "Поиск пользователей по маске имени и фамилии",
        "tags": ["Users"],
        "parameters": [
          {
            "name": "first_name",
            "in": "query",
            "schema": {"type": "string"}
          },
          {
            "name": "last_name",
            "in": "query",
            "schema": {"type": "string"}
          }
        ],
        "responses": {
          "200": {"description": "Список пользователей"}
        }
      }
    },
    "/drivers": {
      "post": {
        "summary": "Регистрация водителя",
        "tags": ["Drivers"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "type": "object",
                "required": ["first_name", "last_name", "car_model", "car_number"],
                "properties": {
                  "first_name": {"type": "string"},
                  "last_name": {"type": "string"},
                  "car_model": {"type": "string"},
                  "car_number": {"type": "string"}
                }
              }
            }
          }
        },
        "responses": {
          "201": {"description": "Водитель зарегистрирован"}
        }
      }
    },
    "/rides": {
      "post": {
        "summary": "Создание заказа поездки",
        "tags": ["Rides"],
        "security": [{"bearerAuth": []}],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "type": "object",
                "required": ["user_id", "start_address", "end_address"],
                "properties": {
                  "user_id": {"type": "string"},
                  "start_address": {"type": "string"},
                  "end_address": {"type": "string"}
                }
              }
            }
          }
        },
        "responses": {
          "201": {"description": "Заказ создан"},
          "401": {"description": "Не авторизован"}
        }
      }
    },
    "/rides/active": {
      "get": {
        "summary": "Получение активных заказов",
        "tags": ["Rides"],
        "security": [{"bearerAuth": []}],
        "responses": {
          "200": {"description": "Список активных заказов"}
        }
      }
    },
    "/rides/{ride_id}/accept": {
      "post": {
        "summary": "Принятие заказа водителем",
        "tags": ["Rides"],
        "security": [{"bearerAuth": []}],
        "parameters": [
          {
            "name": "ride_id",
            "in": "path",
            "required": true,
            "schema": {"type": "string"}
          }
        ],
        "responses": {
          "200": {"description": "Заказ принят"}
        }
      }
    },
    "/users/{user_id}/rides": {
      "get": {
        "summary": "Получение истории поездок пользователя",
        "tags": ["Rides"],
        "parameters": [
          {
            "name": "user_id",
            "in": "path",
            "required": true,
            "schema": {"type": "string"}
          }
        ],
        "responses": {
          "200": {"description": "История поездок"}
        }
      }
    },
    "/rides/{ride_id}/complete": {
      "post": {
        "summary": "Завершение поездки",
        "tags": ["Rides"],
        "security": [{"bearerAuth": []}],
        "parameters": [
          {
            "name": "ride_id",
            "in": "path",
            "required": true,
            "schema": {"type": "string"}
          }
        ],
        "responses": {
          "200": {"description": "Поездка завершена"}
        }
      }
    },
    "/auth/login": {
      "post": {
        "summary": "Аутентификация пользователя",
        "tags": ["Auth"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "type": "object",
                "required": ["login", "password"],
                "properties": {
                  "login": {"type": "string"},
                  "password": {"type": "string"}
                }
              }
            }
          }
        },
        "responses": {
          "200": {"description": "Успешный вход"},
          "401": {"description": "Неверные учетные данные"}
        }
      }
    },
    "/auth/register": {
      "post": {
        "summary": "Регистрация нового пользователя",
        "tags": ["Auth"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "type": "object",
                "required": ["login", "password", "first_name", "last_name", "email"],
                "properties": {
                  "login": {"type": "string"},
                  "password": {"type": "string"},
                  "first_name": {"type": "string"},
                  "last_name": {"type": "string"},
                  "email": {"type": "string", "format": "email"}
                }
              }
            }
          }
        },
        "responses": {
          "201": {"description": "Пользователь зарегистрирован"}
        }
      }
    }
  },
  "components": {
    "securitySchemes": {
      "bearerAuth": {
        "type": "http",
        "scheme": "bearer",
        "bearerFormat": "JWT"
      }
    }
  }
})";
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    // Устанавливаем Content-Type для JSON
    request.GetHttpResponse().SetContentType("application/json");
    return openapi_json;
}

} // namespace handlers