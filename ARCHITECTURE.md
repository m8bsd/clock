# Web Server Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        CLIENT REQUEST                        │
│                   (Browser, curl, etc.)                      │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      SOCKET LAYER                            │
│  • TCP Connection (port 8080)                                │
│  • recv() - Read raw HTTP request                            │
│  • send() - Send HTTP response                               │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   REQUEST PARSER                             │
│  parse_request()                                             │
│    ├─ Extract HTTP method (GET/POST/PUT/DELETE)             │
│    ├─ Extract path (/api/users)                             │
│    ├─ Parse query string (?name=value)                      │
│    ├─ Extract headers                                        │
│    └─ Extract body (for POST/PUT)                           │
│                                                              │
│  Output: HttpRequest struct                                  │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  MIDDLEWARE CHAIN                            │
│  (Executed in order of registration)                         │
│                                                              │
│  ┌────────────────────────────────────────────┐             │
│  │  1. Logger Middleware                      │             │
│  │     • Log timestamp, method, path          │             │
│  │     • Return: true (continue)              │             │
│  └────────────────┬───────────────────────────┘             │
│                   │                                          │
│                   ▼                                          │
│  ┌────────────────────────────────────────────┐             │
│  │  2. Auth Middleware                        │             │
│  │     • Check if route is protected          │             │
│  │     • Verify Authorization header          │             │
│  │     • Return: true/false (continue/stop)   │             │
│  └────────────────┬───────────────────────────┘             │
│                   │                                          │
│                   ▼                                          │
│  ┌────────────────────────────────────────────┐             │
│  │  3. CORS Middleware                        │             │
│  │     • Add CORS headers                     │             │
│  │     • Return: true (continue)              │             │
│  └────────────────┬───────────────────────────┘             │
│                   │                                          │
│  If any middleware returns false → stop here                │
└───────────────────┬─────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────────┐
│                    ROUTER                                    │
│  find_handler()                                              │
│                                                              │
│  Registered Routes:                                          │
│    GET    /              → handle_home()                     │
│    GET    /api/hello     → handle_hello()                    │
│    GET    /api/time      → handle_time()                     │
│    GET    /api/users     → handle_users_list()               │
│    POST   /api/users     → handle_user_create()              │
│    GET    /api/users/:id → handle_user_get()                 │
│    DELETE /api/users/:id → handle_user_delete()              │
│    GET    /admin         → handle_admin()                    │
│    *      *              → handle_not_found()                │
│                                                              │
│  Route Matching:                                             │
│    1. Check method matches                                   │
│    2. Check path (exact or pattern match)                    │
│    3. Return handler function pointer                        │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  ROUTE HANDLER                               │
│  RouteHandler function is called                             │
│                                                              │
│  Input:  HttpRequest  *req                                   │
│  Output: HttpResponse *res                                   │
│                                                              │
│  Handler responsibilities:                                   │
│    • Parse request data                                      │
│    • Perform business logic                                  │
│    • Prepare response                                        │
│    • Set status code                                         │
│    • Set content type                                        │
│    • Set response body                                       │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 RESPONSE BUILDER                             │
│  send_response()                                             │
│                                                              │
│  Build HTTP response:                                        │
│    HTTP/1.1 {status_code} {status_text}                      │
│    Content-Type: {content_type}                              │
│    Content-Length: {body_length}                             │
│    Connection: close                                         │
│                                                              │
│    {response_body}                                           │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    SEND TO CLIENT                            │
│  • send() system call                                        │
│  • close() connection                                        │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
                     CLIENT RECEIVES RESPONSE


## Data Flow Example: GET /api/users/123

1. Client sends:
   GET /api/users/123 HTTP/1.1
   Host: localhost:8080

2. Socket receives → parse_request()
   req.method = GET
   req.path = "/api/users/123"

3. Middleware chain executes:
   logger_middleware()  → logs request → returns true
   auth_middleware()    → no auth needed → returns true
   cors_middleware()    → adds headers → returns true

4. Router matches:
   GET /api/users/:id → handle_user_get()

5. Handler executes:
   • Extracts user_id = 123 from path
   • Generates JSON response
   • Sets status = 200
   • Sets content_type = "application/json"

6. Response sent:
   HTTP/1.1 200 OK
   Content-Type: application/json
   Content-Length: 67
   Connection: close

   {"id": 123, "name": "User 123", "email": "user123@example.com"}


## Key Data Structures

### HttpRequest
┌─────────────────────────────┐
│ HttpMethod method           │
│ char path[256]              │
│ char query_string[512]      │
│ char body[2048]             │
│ int body_length             │
│ char headers[1024]          │
└─────────────────────────────┘

### HttpResponse
┌─────────────────────────────┐
│ int status_code             │
│ char content_type[64]       │
│ char body[4096]             │
│ int body_length             │
└─────────────────────────────┘

### Route
┌─────────────────────────────┐
│ HttpMethod method           │
│ char path[256]              │
│ RouteHandler handler        │ ──→ Function pointer
└─────────────────────────────┘

### Server
┌─────────────────────────────┐
│ Route routes[50]            │
│ int route_count             │
│ Middleware middleware[10]   │
│ int middleware_count        │
└─────────────────────────────┘


## Function Pointer Pattern

### Middleware
typedef bool (*Middleware)(HttpRequest*, HttpResponse*);

Example usage:
  bool logger_middleware(HttpRequest* req, HttpResponse* res) {
      printf("Request: %s %s\n", method, path);
      return true;  // Continue processing
  }

  register_middleware(logger_middleware);

### Route Handler
typedef void (*RouteHandler)(HttpRequest*, HttpResponse*);

Example usage:
  void handle_users(HttpRequest* req, HttpResponse* res) {
      set_json_response(res, 200, "{...}");
  }

  register_route(GET, "/api/users", handle_users);


## Extensibility Points

1. Add new routes:
   register_route(METHOD, "/path", handler_function)

2. Add new middleware:
   register_middleware(middleware_function)

3. Custom response helpers:
   set_json_response()
   set_html_response()
   set_text_response()

4. Pattern matching:
   Support for :id parameters in routes
   /api/users/:id matches /api/users/123


## Concurrency Model

Currently: SINGLE-THREADED
  • One request at a time
  • Blocking I/O
  • Simple but not scalable

For production, consider:
  • Thread pool
  • epoll/kqueue for async I/O
  • Process forking
  • Event-driven architecture
