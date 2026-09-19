#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_ROUTES 50
#define MAX_MIDDLEWARE 10

// HTTP Methods
typedef enum {
    GET,
    POST,
    PUT,
    DELETE,
    UNSUPPORTED
} HttpMethod;

// Request structure
typedef struct {
    HttpMethod method;
    char path[256];
    char query_string[512];
    char body[2048];
    int body_length;
    char headers[1024];
} HttpRequest;

// Response structure
typedef struct {
    int status_code;
    char content_type[64];
    char body[4096];
    int body_length;
} HttpResponse;

// Handler function type
typedef void (*RouteHandler)(HttpRequest*, HttpResponse*);

// Middleware function type (returns true to continue, false to stop)
typedef bool (*Middleware)(HttpRequest*, HttpResponse*);

// Route structure
typedef struct {
    HttpMethod method;
    char path[256];
    RouteHandler handler;
} Route;

// Server structure
typedef struct {
    Route routes[MAX_ROUTES];
    int route_count;
    Middleware middleware[MAX_MIDDLEWARE];
    int middleware_count;
} Server;

Server server = {0};

// ============= Utility Functions =============

HttpMethod parse_method(const char* method_str) {
    if (strcmp(method_str, "GET") == 0) return GET;
    if (strcmp(method_str, "POST") == 0) return POST;
    if (strcmp(method_str, "PUT") == 0) return PUT;
    if (strcmp(method_str, "DELETE") == 0) return DELETE;
    return UNSUPPORTED;
}

const char* method_to_string(HttpMethod method) {
    switch(method) {
        case GET: return "GET";
        case POST: return "POST";
        case PUT: return "PUT";
        case DELETE: return "DELETE";
        default: return "UNSUPPORTED";
    }
}

void parse_request(const char* raw_request, HttpRequest* req) {
    char method_str[16];
    char full_path[512];
    
    sscanf(raw_request, "%s %s", method_str, full_path);
    req->method = parse_method(method_str);
    
    // Parse path and query string
    char* query_start = strchr(full_path, '?');
    if (query_start) {
        *query_start = '\0';
        strncpy(req->path, full_path, sizeof(req->path) - 1);
        strncpy(req->query_string, query_start + 1, sizeof(req->query_string) - 1);
    } else {
        strncpy(req->path, full_path, sizeof(req->path) - 1);
        req->query_string[0] = '\0';
    }
    
    // Parse body for POST/PUT requests
    const char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        strncpy(req->body, body_start, sizeof(req->body) - 1);
        req->body_length = strlen(req->body);
    }
    
    // Copy headers
    strncpy(req->headers, raw_request, sizeof(req->headers) - 1);
}

void init_response(HttpResponse* res) {
    res->status_code = 200;
    strcpy(res->content_type, "text/plain");
    res->body[0] = '\0';
    res->body_length = 0;
}

void set_json_response(HttpResponse* res, int status, const char* json) {
    res->status_code = status;
    strcpy(res->content_type, "application/json");
    strncpy(res->body, json, sizeof(res->body) - 1);
    res->body_length = strlen(res->body);
}

void set_text_response(HttpResponse* res, int status, const char* text) {
    res->status_code = status;
    strcpy(res->content_type, "text/plain");
    strncpy(res->body, text, sizeof(res->body) - 1);
    res->body_length = strlen(res->body);
}

void set_html_response(HttpResponse* res, int status, const char* html) {
    res->status_code = status;
    strcpy(res->content_type, "text/html");
    strncpy(res->body, html, sizeof(res->body) - 1);
    res->body_length = strlen(res->body);
}

const char* get_status_text(int code) {
    switch(code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

// ============= Middleware Functions =============

bool logger_middleware(HttpRequest* req, HttpResponse* res) {
    time_t now;
    time(&now);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remove newline
    
    printf("[%s] %s %s", time_str, method_to_string(req->method), req->path);
    if (req->query_string[0]) {
        printf("?%s", req->query_string);
    }
    printf("\n");
    
    return true; // Continue to next middleware/handler
}

bool cors_middleware(HttpRequest* req, HttpResponse* res) {
    // This would add CORS headers in a real implementation
    // For now, just demonstrate the concept
    return true;
}

bool auth_middleware(HttpRequest* req, HttpResponse* res) {
    // Check for protected routes
    if (strncmp(req->path, "/admin", 6) == 0) {
        // Look for Authorization header (simplified)
        if (strstr(req->headers, "Authorization:") == NULL) {
            set_json_response(res, 401, "{\"error\": \"Unauthorized\"}");
            return false; // Stop processing
        }
    }
    return true;
}

// ============= Route Handlers =============

void handle_home(HttpRequest* req, HttpResponse* res) {
    const char* html = 
        "<!DOCTYPE html>"
        "<html><head><title>C Web Server</title></head>"
        "<body>"
        "<h1>Welcome to the C Web Server!</h1>"
        "<p>Available endpoints:</p>"
        "<ul>"
        "<li>GET / - This page</li>"
        "<li>GET /api/hello - Hello JSON</li>"
        "<li>GET /api/time - Current time</li>"
        "<li>GET /api/users - List users</li>"
        "<li>POST /api/users - Create user</li>"
        "<li>GET /api/users/123 - Get specific user</li>"
        "<li>DELETE /api/users/123 - Delete user</li>"
        "<li>GET /admin - Protected route (requires auth)</li>"
        "</ul>"
        "</body></html>";
    
    set_html_response(res, 200, html);
}

void handle_hello(HttpRequest* req, HttpResponse* res) {
    const char* name = "Guest";
    
    // Parse query parameter
    if (req->query_string[0]) {
        char* name_param = strstr(req->query_string, "name=");
        if (name_param) {
            static char name_buffer[64];
            sscanf(name_param, "name=%63s", name_buffer);
            name = name_buffer;
        }
    }
    
    char json[256];
    snprintf(json, sizeof(json), 
             "{\"message\": \"Hello, %s!\", \"timestamp\": %ld}", 
             name, time(NULL));
    
    set_json_response(res, 200, json);
}

void handle_time(HttpRequest* req, HttpResponse* res) {
    time_t now = time(NULL);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    
    char json[256];
    snprintf(json, sizeof(json), 
             "{\"current_time\": \"%s\", \"unix_timestamp\": %ld}", 
             time_str, now);
    
    set_json_response(res, 200, json);
}

void handle_users_list(HttpRequest* req, HttpResponse* res) {
    const char* json = 
        "{"
        "  \"users\": ["
        "    {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},"
        "    {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"},"
        "    {\"id\": 3, \"name\": \"Charlie\", \"email\": \"charlie@example.com\"}"
        "  ],"
        "  \"count\": 3"
        "}";
    
    set_json_response(res, 200, json);
}

void handle_user_create(HttpRequest* req, HttpResponse* res) {
    // In a real app, you'd parse the JSON body and save to database
    printf("Received POST body: %s\n", req->body);
    
    const char* json = 
        "{"
        "  \"id\": 4,"
        "  \"name\": \"New User\","
        "  \"email\": \"newuser@example.com\","
        "  \"created\": true"
        "}";
    
    set_json_response(res, 201, json);
}

void handle_user_get(HttpRequest* req, HttpResponse* res) {
    // Extract user ID from path /api/users/123
    int user_id = 0;
    sscanf(req->path, "/api/users/%d", &user_id);
    
    if (user_id > 0 && user_id <= 3) {
        char json[256];
        snprintf(json, sizeof(json),
                 "{\"id\": %d, \"name\": \"User %d\", \"email\": \"user%d@example.com\"}",
                 user_id, user_id, user_id);
        set_json_response(res, 200, json);
    } else {
        set_json_response(res, 404, "{\"error\": \"User not found\"}");
    }
}

void handle_user_delete(HttpRequest* req, HttpResponse* res) {
    int user_id = 0;
    sscanf(req->path, "/api/users/%d", &user_id);
    
    char json[128];
    snprintf(json, sizeof(json),
             "{\"message\": \"User %d deleted\", \"success\": true}",
             user_id);
    set_json_response(res, 200, json);
}

void handle_admin(HttpRequest* req, HttpResponse* res) {
    set_json_response(res, 200, "{\"message\": \"Welcome to admin panel\"}");
}

void handle_not_found(HttpRequest* req, HttpResponse* res) {
    set_json_response(res, 404, "{\"error\": \"Route not found\"}");
}

// ============= Routing System =============

void register_route(HttpMethod method, const char* path, RouteHandler handler) {
    if (server.route_count < MAX_ROUTES) {
        server.routes[server.route_count].method = method;
        strncpy(server.routes[server.route_count].path, path, 
                sizeof(server.routes[server.route_count].path) - 1);
        server.routes[server.route_count].handler = handler;
        server.route_count++;
    }
}

void register_middleware(Middleware middleware) {
    if (server.middleware_count < MAX_MIDDLEWARE) {
        server.middleware[server.middleware_count++] = middleware;
    }
}

bool path_matches(const char* route_path, const char* req_path) {
    // Exact match
    if (strcmp(route_path, req_path) == 0) {
        return true;
    }
    
    // Pattern match for /api/users/:id style routes
    if (strstr(route_path, "/:id")) {
        char pattern[256];
        strncpy(pattern, route_path, sizeof(pattern) - 1);
        char* placeholder = strstr(pattern, "/:id");
        if (placeholder) {
            *placeholder = '\0';
            if (strncmp(pattern, req_path, strlen(pattern)) == 0) {
                return true;
            }
        }
    }
    
    return false;
}

RouteHandler find_handler(HttpRequest* req) {
    for (int i = 0; i < server.route_count; i++) {
        if (server.routes[i].method == req->method &&
            path_matches(server.routes[i].path, req->path)) {
            return server.routes[i].handler;
        }
    }
    return handle_not_found;
}

void handle_request(HttpRequest* req, HttpResponse* res) {
    // Execute middleware chain
    for (int i = 0; i < server.middleware_count; i++) {
        if (!server.middleware[i](req, res)) {
            return; // Middleware stopped the request
        }
    }
    
    // Find and execute handler
    RouteHandler handler = find_handler(req);
    handler(req, res);
}

// ============= Server Setup =============

void setup_routes() {
    // Register middleware (order matters!)
    register_middleware(logger_middleware);
    register_middleware(auth_middleware);
    register_middleware(cors_middleware);
    
    // Register routes
    register_route(GET, "/", handle_home);
    register_route(GET, "/api/hello", handle_hello);
    register_route(GET, "/api/time", handle_time);
    register_route(GET, "/api/users", handle_users_list);
    register_route(POST, "/api/users", handle_user_create);
    register_route(GET, "/api/users/:id", handle_user_get);
    register_route(DELETE, "/api/users/:id", handle_user_delete);
    register_route(GET, "/admin", handle_admin);
}

void send_response(int client_sock, HttpResponse* res) {
    char response[8192];
    int len = snprintf(response, sizeof(response),
                      "HTTP/1.1 %d %s\r\n"
                      "Content-Type: %s\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "\r\n"
                      "%s",
                      res->status_code,
                      get_status_text(res->status_code),
                      res->content_type,
                      res->body_length,
                      res->body);
    
    send(client_sock, response, len, 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // Initialize server
    setup_routes();
    
    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }
    
    // Listen for connections
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }
    
    printf("Server listening on port %d...\n", PORT);
    printf("Visit http://localhost:%d in your browser\n\n", PORT);
    
    // Main server loop
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Read request
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            // Parse request
            HttpRequest req = {0};
            HttpResponse res;
            init_response(&res);
            
            parse_request(buffer, &req);
            
            // Handle request
            handle_request(&req, &res);
            
            // Send response
            send_response(client_sock, &res);
        }
        
        close(client_sock);
    }
    
    close(server_sock);
    return 0;
}
