# C Web Server

A lightweight, educational web server built using only the C standard library that demonstrates:
- HTTP request parsing and routing
- RESTful JSON APIs
- Middleware pattern
- Route handlers
- Query parameter parsing

## Features

### 🎯 Routing System
- Method-based routing (GET, POST, PUT, DELETE)
- Pattern matching for dynamic routes (e.g., `/api/users/:id`)
- Exact path matching
- Automatic 404 handling

### 🔧 Middleware
- **Logger**: Logs all incoming requests with timestamps
- **Authentication**: Protects routes (e.g., `/admin`)
- **CORS**: Placeholder for cross-origin support
- Middleware chain execution (order matters!)

### 📡 JSON APIs
- RESTful endpoints with JSON responses
- Proper Content-Type headers
- HTTP status codes (200, 201, 404, 401, etc.)
- Request body parsing

### 🛣️ Available Routes

#### General
- `GET /` - HTML home page with route listing

#### API Endpoints
- `GET /api/hello?name=YourName` - Personalized greeting
- `GET /api/time` - Current server time
- `GET /api/users` - List all users
- `POST /api/users` - Create a new user
- `GET /api/users/123` - Get specific user by ID
- `DELETE /api/users/123` - Delete user by ID

#### Protected Routes
- `GET /admin` - Requires Authorization header

## Building and Running

### Compile
```bash
make
```

Or manually:
```bash
gcc -Wall -Wextra -std=c11 -o webserver webserver.c
```

### Run
```bash
make run
```

Or:
```bash
./webserver
```

The server will start on `http://localhost:8080`

### Clean
```bash
make clean
```

## Testing the Server

### Using curl

**Basic GET request:**
```bash
curl http://localhost:8080/
```

**JSON API request:**
```bash
curl http://localhost:8080/api/hello?name=Alice
```

**Get current time:**
```bash
curl http://localhost:8080/api/time
```

**List users:**
```bash
curl http://localhost:8080/api/users
```

**Get specific user:**
```bash
curl http://localhost:8080/api/users/1
```

**Create user (POST):**
```bash
curl -X POST http://localhost:8080/api/users \
  -H "Content-Type: application/json" \
  -d '{"name":"John","email":"john@example.com"}'
```

**Delete user:**
```bash
curl -X DELETE http://localhost:8080/api/users/1
```

**Access protected route (will fail):**
```bash
curl http://localhost:8080/admin
# Returns: {"error": "Unauthorized"}
```

**Access with auth (simplified demo):**
```bash
curl http://localhost:8080/admin -H "Authorization: Bearer token"
# Returns: {"message": "Welcome to admin panel"}
```

### Using a Browser

Simply open: `http://localhost:8080`

You'll see an HTML page listing all available endpoints.

## Architecture

### Request Flow
```
1. Client Connection
   ↓
2. Parse HTTP Request → HttpRequest struct
   ↓
3. Execute Middleware Chain
   ↓
4. Route Matching
   ↓
5. Execute Handler → HttpResponse struct
   ↓
6. Send HTTP Response
   ↓
7. Close Connection
```

### Key Components

#### HttpRequest
```c
typedef struct {
    HttpMethod method;      // GET, POST, PUT, DELETE
    char path[256];         // Request path
    char query_string[512]; // Query parameters
    char body[2048];        // Request body
    int body_length;        // Body size
    char headers[1024];     // Raw headers
} HttpRequest;
```

#### HttpResponse
```c
typedef struct {
    int status_code;        // 200, 404, etc.
    char content_type[64];  // "application/json", etc.
    char body[4096];        // Response body
    int body_length;        // Body size
} HttpResponse;
```

#### Middleware
```c
typedef bool (*Middleware)(HttpRequest*, HttpResponse*);
// Returns true to continue, false to stop processing
```

#### Route Handler
```c
typedef void (*RouteHandler)(HttpRequest*, HttpResponse*);
```

## Code Structure

```
webserver.c
├── Utility Functions
│   ├── parse_method()
│   ├── parse_request()
│   ├── set_json_response()
│   └── set_html_response()
│
├── Middleware Functions
│   ├── logger_middleware()
│   ├── cors_middleware()
│   └── auth_middleware()
│
├── Route Handlers
│   ├── handle_home()
│   ├── handle_hello()
│   ├── handle_users_list()
│   ├── handle_user_create()
│   └── handle_not_found()
│
├── Routing System
│   ├── register_route()
│   ├── register_middleware()
│   ├── find_handler()
│   └── handle_request()
│
└── Main Server Loop
    ├── setup_routes()
    ├── socket creation
    ├── bind and listen
    └── accept and handle connections
```

## Extending the Server

### Adding a New Route

```c
// 1. Create a handler function
void handle_my_route(HttpRequest* req, HttpResponse* res) {
    set_json_response(res, 200, "{\"message\": \"Hello!\"}");
}

// 2. Register in setup_routes()
void setup_routes() {
    // ... existing routes ...
    register_route(GET, "/my-route", handle_my_route);
}
```

### Adding New Middleware

```c
// 1. Create middleware function
bool my_middleware(HttpRequest* req, HttpResponse* res) {
    // Do something before the handler
    printf("Custom middleware executing\n");
    return true; // Continue to next middleware/handler
}

// 2. Register in setup_routes()
void setup_routes() {
    register_middleware(my_middleware);
    // ... other registrations ...
}
```

### Parsing Query Parameters

```c
void handle_search(HttpRequest* req, HttpResponse* res) {
    char query[128] = "default";
    
    if (req->query_string[0]) {
        char* q = strstr(req->query_string, "q=");
        if (q) {
            sscanf(q, "q=%127s", query);
        }
    }
    
    char json[256];
    snprintf(json, sizeof(json), 
             "{\"query\": \"%s\", \"results\": []}", query);
    set_json_response(res, 200, json);
}
```

### Parsing JSON Request Body

```c
void handle_json_body(HttpRequest* req, HttpResponse* res) {
    // Simple JSON parsing (use a library for production)
    char name[64] = "";
    
    char* name_field = strstr(req->body, "\"name\":");
    if (name_field) {
        sscanf(name_field, "\"name\":\"%63[^\"]\"", name);
    }
    
    printf("Parsed name: %s\n", name);
    set_json_response(res, 200, "{\"received\": true}");
}
```

## Limitations

This is an educational server. For production use, consider:

- ❌ Not thread-safe (single-threaded)
- ❌ No HTTPS/TLS support
- ❌ Limited buffer sizes
- ❌ No proper JSON parsing library
- ❌ No persistent data storage
- ❌ Basic error handling
- ❌ No request timeout handling
- ❌ No compression support

## Educational Goals

This server demonstrates:
- ✅ Socket programming in C
- ✅ HTTP protocol fundamentals
- ✅ Routing and middleware patterns
- ✅ RESTful API design
- ✅ Function pointers and callbacks
- ✅ String parsing in C
- ✅ Struct-based architecture

## Requirements

- GCC compiler with C11 support
- POSIX-compliant system (Linux, macOS, WSL)
- Standard C library
- POSIX sockets

## License

MIT License - Feel free to use for learning and educational purposes.

## Further Reading

- [RFC 2616 - HTTP/1.1](https://tools.ietf.org/html/rfc2616)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [REST API Design Best Practices](https://restfulapi.net/)
