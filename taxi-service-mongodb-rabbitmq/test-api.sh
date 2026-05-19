#!/bin/bash

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================="
echo "     TAXI API TEST SCRIPT"
echo "========================================="

BASE_URL="http://localhost:8080"

# 1. Создание пользователя
echo -e "\n${YELLOW}[1/10] Creating User...${NC}"
TIMESTAMP=$(date +%s)
USER_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/users \
  -H "Content-Type: application/json" \
  -d "{
    \"login\": \"rider_$TIMESTAMP\",
    \"password\": \"pass123\",
    \"first_name\": \"Rider\",
    \"last_name\": \"Test\",
    \"email\": \"rider_$TIMESTAMP@example.com\"
  }")
USER_ID=$(echo $USER_RESPONSE | jq -r '.id')
if [ "$USER_ID" != "null" ] && [ -n "$USER_ID" ]; then
    echo -e "${GREEN}✓ User created with ID: $USER_ID${NC}"
else
    echo -e "${RED}✗ Failed to create user${NC}"
    exit 1
fi

# 2. Аутентификация пользователя
echo -e "\n${YELLOW}[2/10] Authenticating User...${NC}"
USER_LOGIN=$(echo $USER_RESPONSE | jq -r '.login')
LOGIN_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d "{\"login\":\"$USER_LOGIN\",\"password\":\"pass123\"}")
USER_TOKEN=$(echo $LOGIN_RESPONSE | jq -r '.token')
if [ "$USER_TOKEN" != "null" ] && [ -n "$USER_TOKEN" ]; then
    echo -e "${GREEN}✓ User authenticated${NC}"
else
    echo -e "${RED}✗ Authentication failed${NC}"
    exit 1
fi

# 3. Регистрация водителя (с логином и паролем)
echo -e "\n${YELLOW}[3/10] Registering Driver...${NC}"
DRIVER_LOGIN="driver_$TIMESTAMP"
DRIVER_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/drivers \
  -H "Content-Type: application/json" \
  -d "{
    \"login\": \"$DRIVER_LOGIN\",
    \"first_name\": \"Driver\",
    \"last_name\": \"Test\",
    \"car_model\": \"Tesla Model 3\",
    \"car_number\": \"DRV_$TIMESTAMP\",
    \"password\": \"driverpass\"
  }")
DRIVER_ID=$(echo $DRIVER_RESPONSE | jq -r '.id')
if [ "$DRIVER_ID" != "null" ] && [ -n "$DRIVER_ID" ]; then
    echo -e "${GREEN}✓ Driver registered with ID: $DRIVER_ID${NC}"
else
    echo -e "${RED}✗ Failed to register driver${NC}"
    echo "$DRIVER_RESPONSE"
    exit 1
fi

# 4. Аутентификация водителя
echo -e "\n${YELLOW}[4/10] Authenticating Driver...${NC}"
DRIVER_AUTH_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d "{\"login\":\"$DRIVER_LOGIN\",\"password\":\"driverpass\"}")
DRIVER_TOKEN=$(echo $DRIVER_AUTH_RESPONSE | jq -r '.token')
if [ "$DRIVER_TOKEN" != "null" ] && [ -n "$DRIVER_TOKEN" ]; then
    echo -e "${GREEN}✓ Driver authenticated${NC}"
else
    echo -e "${RED}✗ Driver authentication failed${NC}"
    echo "Response: $DRIVER_AUTH_RESPONSE"
fi

# 5. Создание заказа (пользователь)
echo -e "\n${YELLOW}[5/10] Creating Ride...${NC}"
RIDE_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/rides \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER_TOKEN" \
  -d "{
    \"user_id\": \"$USER_ID\",
    \"start_address\": \"ул. Ленина, 10\",
    \"end_address\": \"ул. Пушкина, 20\"
  }")
RIDE_ID=$(echo $RIDE_RESPONSE | jq -r '.id')
if [ "$RIDE_ID" != "null" ] && [ -n "$RIDE_ID" ]; then
    echo -e "${GREEN}✓ Ride created with ID: $RIDE_ID${NC}"
else
    echo -e "${RED}✗ Failed to create ride${NC}"
    echo "$RIDE_RESPONSE"
fi

# 6. Получение активных заказов
echo -e "\n${YELLOW}[6/10] Getting Active Rides...${NC}"
ACTIVE_RESPONSE=$(curl -s -X GET "$BASE_URL/api/v1/rides/active" \
  -H "Authorization: Bearer $USER_TOKEN")
ACTIVE_COUNT=$(echo $ACTIVE_RESPONSE | jq 'length' 2>/dev/null)
if [ "$ACTIVE_COUNT" -gt 0 ] 2>/dev/null; then
    echo -e "${GREEN}✓ Found $ACTIVE_COUNT active rides${NC}"
else
    echo -e "${YELLOW}! No active rides found${NC}"
fi

# 7. Принятие заказа водителем
if [ -n "$DRIVER_TOKEN" ] && [ "$DRIVER_TOKEN" != "null" ]; then
    echo -e "\n${YELLOW}[7/10] Accepting Ride by Driver...${NC}"
    ACCEPT_RESPONSE=$(curl -s -X POST "$BASE_URL/api/v1/rides/$RIDE_ID/accept" \
      -H "Authorization: Bearer $DRIVER_TOKEN")
    ACCEPT_STATUS=$(echo $ACCEPT_RESPONSE | jq -r '.error // "success"')
    if [ "$ACCEPT_STATUS" = "success" ]; then
        echo -e "${GREEN}✓ Ride accepted${NC}"
    else
        echo -e "${YELLOW}! Accept error: $ACCEPT_STATUS${NC}"
    fi
else
    echo -e "\n${YELLOW}[7/10] Skipping Accept Ride - driver not authenticated${NC}"
fi

# 8. История поездок пользователя
echo -e "\n${YELLOW}[8/10] Getting User Ride History...${NC}"
HISTORY_RESPONSE=$(curl -s -X GET "$BASE_URL/api/v1/users/$USER_ID/rides")
HISTORY_COUNT=$(echo $HISTORY_RESPONSE | jq 'length' 2>/dev/null)
if [ "$HISTORY_COUNT" -gt 0 ] 2>/dev/null; then
    echo -e "${GREEN}✓ Found $HISTORY_COUNT rides in history${NC}"
else
    echo -e "${YELLOW}! No rides in history yet${NC}"
fi

# 9. Завершение поездки
if [ -n "$DRIVER_TOKEN" ] && [ "$DRIVER_TOKEN" != "null" ]; then
    echo -e "\n${YELLOW}[9/10] Completing Ride...${NC}"
    COMPLETE_RESPONSE=$(curl -s -X POST "$BASE_URL/api/v1/rides/$RIDE_ID/complete" \
      -H "Authorization: Bearer $DRIVER_TOKEN")
    COMPLETE_STATUS=$(echo $COMPLETE_RESPONSE | jq -r '.error // "success"')
    if [ "$COMPLETE_STATUS" = "success" ]; then
        echo -e "${GREEN}✓ Ride completed${NC}"
    else
        echo -e "${YELLOW}! Complete error: $COMPLETE_STATUS${NC}"
    fi
else
    echo -e "\n${YELLOW}[9/10] Skipping Complete Ride - driver not authenticated${NC}"
fi

# 10. Регистрация через auth
echo -e "\n${YELLOW}[10/10] Register via Auth Endpoint...${NC}"
AUTH_REG_RESPONSE=$(curl -s -X POST $BASE_URL/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d "{
    \"login\": \"auth_user_$TIMESTAMP\",
    \"password\": \"auth123\",
    \"first_name\": \"Auth\",
    \"last_name\": \"User\",
    \"email\": \"auth_$TIMESTAMP@example.com\"
  }")
AUTH_TOKEN=$(echo $AUTH_REG_RESPONSE | jq -r '.token')
if [ "$AUTH_TOKEN" != "null" ] && [ -n "$AUTH_TOKEN" ]; then
    echo -e "${GREEN}✓ Auth registration successful${NC}"
else
    echo -e "${RED}✗ Auth registration failed${NC}"
fi

echo -e "\n========================================="
echo -e "${GREEN}✓ TEST COMPLETED${NC}"
echo -e "========================================="
echo -e "User ID: $USER_ID"
echo -e "Driver ID: $DRIVER_ID"
echo -e "Ride ID: $RIDE_ID"
echo -e "Swagger UI: $BASE_URL/docs"
echo -e "========================================="