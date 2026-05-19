#!/bin/bash

echo "=== Building Docker image ==="
docker-compose build

echo ""
echo "=== Starting service ==="
docker-compose up -d

echo ""
echo "=== Waiting for service to start (10 seconds) ==="
sleep 10

echo ""
echo "=== Testing /ping endpoint ==="
curl -s http://localhost:8080/ping
echo ""

echo ""
echo "=== Testing user registration ==="
curl -s -X POST http://localhost:8080/api/v1/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "docker_test",
    "password": "test123",
    "first_name": "Docker",
    "last_name": "Test",
    "email": "docker@test.com"
  }'
echo ""

echo ""
echo "=== Testing login ==="
curl -s -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"docker_test","password":"test123"}'
echo ""

echo ""
echo "=== Service logs ==="
docker-compose logs --tail=20

echo ""
echo "=== To stop service: docker-compose down ==="
