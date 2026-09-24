#!/bin/bash

# Test script for the C web server
# Usage: ./test_server.sh

SERVER="http://localhost:8080"

echo "================================"
echo "Testing C Web Server"
echo "================================"
echo ""

# Test 1: Home page
echo "1. Testing GET / (Home page)"
curl -s "$SERVER/" | head -n 5
echo ""
echo "---"
echo ""

# Test 2: Hello API
echo "2. Testing GET /api/hello"
curl -s "$SERVER/api/hello"
echo ""
echo ""

# Test 3: Hello API with name parameter
echo "3. Testing GET /api/hello?name=Alice"
curl -s "$SERVER/api/hello?name=Alice"
echo ""
echo ""

# Test 4: Time API
echo "4. Testing GET /api/time"
curl -s "$SERVER/api/time"
echo ""
echo ""

# Test 5: List users
echo "5. Testing GET /api/users"
curl -s "$SERVER/api/users"
echo ""
echo ""

# Test 6: Get specific user
echo "6. Testing GET /api/users/1"
curl -s "$SERVER/api/users/1"
echo ""
echo ""

# Test 7: Create user (POST)
echo "7. Testing POST /api/users"
curl -s -X POST "$SERVER/api/users" \
  -H "Content-Type: application/json" \
  -d '{"name":"John","email":"john@example.com"}'
echo ""
echo ""

# Test 8: Delete user
echo "8. Testing DELETE /api/users/2"
curl -s -X DELETE "$SERVER/api/users/2"
echo ""
echo ""

# Test 9: 404 - Not found
echo "9. Testing GET /nonexistent (should be 404)"
curl -s "$SERVER/nonexistent"
echo ""
echo ""

# Test 10: Protected route without auth
echo "10. Testing GET /admin (without auth - should fail)"
curl -s "$SERVER/admin"
echo ""
echo ""

# Test 11: Protected route with auth
echo "11. Testing GET /admin (with auth header)"
curl -s "$SERVER/admin" -H "Authorization: Bearer fake-token"
echo ""
echo ""

echo "================================"
echo "All tests completed!"
echo "================================"
