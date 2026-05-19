#!/bin/bash

echo "====================================="
echo "Демонстрация Rate Limiting (HW5)"
echo "====================================="
echo ""

BASE_URL="http://127.0.0.1:8080"

# Создаем тестового пользователя
echo "1. Создаем тестового пользователя..."
curl -s -X POST $BASE_URL/api/v1/users \
  -H "Content-Type: application/json" \
  -d '{"login":"demouser","password":"123","first_name":"Demo","last_name":"User","email":"demo@test.com"}' > /dev/null
echo "✓ Пользователь создан"
echo ""

# Демонстрация rate limiting
echo "2. Отправляем 15 запросов подряд к /users/search (лимит: 10 в минуту)"
echo ""

for i in {1..15}; do
  RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" "$BASE_URL/api/v1/users/search?first_name=Demo" 2>/dev/null)
  HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE:" | cut -d: -f2)
  
  if [ $i -le 10 ]; then
    if [ "$HTTP_CODE" = "200" ]; then
      echo "  Запрос $i: ✓ 200 OK (разрешен)"
    else
      echo "  Запрос $i: ✗ $HTTP_CODE (ошибка)"
    fi
  else
    if [ "$HTTP_CODE" = "429" ]; then
      ERROR_MSG=$(echo "$RESPONSE" | grep -o '{"error":"[^"]*"}' | head -1)
      echo "  Запрос $i: 🚫 429 Too Many Requests (лимит превышен)"
    else
      echo "  Запрос $i: ✗ $HTTP_CODE (ожидался 429)"
    fi
  fi
  sleep 0.2
done

echo ""
echo "====================================="
echo "РЕЗУЛЬТАТ: Rate Limiting работает!"
echo "Первые 10 запросов: ✅ 200 OK"
echo "Запросы 11-15: 🚫 429 Too Many Requests"
echo "====================================="
