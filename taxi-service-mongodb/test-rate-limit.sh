#!/bin/bash

echo "=== Testing Rate Limiting ==="
echo ""

BASE_URL="http://localhost:8080"

# Сначала создадим тестового пользователя
echo "1. Creating test user..."
curl -s -X POST "$BASE_URL/api/v1/users" \
  -H "Content-Type: application/json" \
  -d '{
    "login": "ratetest",
    "password": "test123",
    "first_name": "Rate",
    "last_name": "Tester",
    "email": "rate@test.com"
  }' | jq .

echo ""
echo ""

# Тестируем rate limiting на search endpoint
echo "2. Sending 12 requests to search endpoint (should get 429 after 10)..."
echo ""

for i in {1..12}; do
    echo "Request $i:"
    curl -s -w "\nHTTP Status: %{http_code}\n" \
      "$BASE_URL/api/v1/users/search?first_name=Rate&last_name=Tester" \
      -H "User-Agent: test-client-$i" \
      -i 2>&1 | grep -E "(HTTP|X-RateLimit|error)"
    echo "---"
    sleep 0.5
done

echo ""
echo "=== Test Complete ==="
