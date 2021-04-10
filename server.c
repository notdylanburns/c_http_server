#include "server.h"

HTTPServer *new_httpserver() {
    HTTPServer *server = malloc(sizeof(HTTPServer));
    if (server == NULL) return NULL;
    server->rt = new_routetable();
    if (server->rt == NULL) return NULL;
    server->running = 1;
    server->error = 0;
    return server;
}

struct ThreadInfo {
    int socketfd;
    HTTPServer *server;
};

void run_server(HTTPServer *server, uint16_t port) {
    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (listener_fd < 1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(port) };

    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if(bind(listener_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    };

    if (listen(listener_fd, 10) < 0) {
        perror("Error whilst listening on socket");
        exit(EXIT_FAILURE);
    }

    int open_socket;

    pthread_t thread_id;
    while (server->running) {
        open_socket = accept(listener_fd, (struct sockaddr *)&address, &addrlen);
        if (open_socket < 0) {
            perror("Socket connection error");
            exit(EXIT_FAILURE);
        }
        
        struct ThreadInfo ti = { .socketfd = open_socket, .server = server };
        int error = pthread_create(&thread_id, NULL, handle_request, &ti);
        if (error != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        } else {
            pthread_join(thread_id, NULL);
        }

    }

}

int send_response(int socketfd, struct HTTPResponse *res) {
    time_t val = time(NULL);
    struct tm *t = gmtime(&val);
    // day, dd mon year hh:mm:ss tmz
    res->date = calloc(30, 1);
    strftime(res->date, 30, "%a, %02d %b %Y %T UTC", t);

    char *responseBuffer = build_httpresponse(res);

    int err;
    err = (send(socketfd, responseBuffer, strlen(responseBuffer), 0) << 8);
    err |= (send(socketfd, res->content, res->content_length, 0));
    send(socketfd, "\r\n", 3, 0);

    destroy_httpresponse(res);

    return err;

}

void *handle_request(void *vargp) {
    struct ThreadInfo *ti = vargp;
    int open_socket = ti->socketfd;
    HTTPServer *server = ti->server;

    struct HTTPRequest *req = build_httprequest(open_socket);
    if (req == NULL) return NULL;
    switch (req->method) {
        case GET:
            printf("Method: GET\n");
            break;
        case POST:
            printf("Method: POST\n");
            break;
        case PUT:
            printf("Method: PUT\n");
            break;
        case DELETE:
            printf("Method: DELETE\n");
            break;
        default:
            printf("Method: UNKNOWN\n");
    }
    printf("Route: %s\nVersion: %s\nHost: %s\nUser Agent: %s\nContent Length: %d\nContent Type: %s\nContent: %s\n\n", req->route, req->version, req->host, req->user_agent, req->content_length, req->content_type, (char *)req->content);


    RouteHandler h = get_handler(server, req->method, req->route);
    if (h == NULL) {
        struct HTTPResponse *res = new_httpresponse();
        
        // Set version to http/1.1
        res->version = calloc(9, 1);
        strcpy(res->version, "HTTP/1.1");
        
        // Set default status
        res->status = NOT_FOUND;
        res->status_msg = calloc(10, 1);
        strcpy(res->status_msg, "NOT FOUND");

        res->server = calloc(strlen(SERVERNAME) + 1, 0);
        strcpy(res->server, SERVERNAME);

        send_response(open_socket, res);
        
    } else {
        struct HTTPResponse *res = new_httpresponse();
        
        // Set version to http/1.1
        res->version = calloc(9, 1);
        strcpy(res->version, "HTTP/1.1");
        
        // Set default status
        res->status = OK;
        res->status_msg = calloc(3, 1);
        strcpy(res->status_msg, "OK");

        res->server = calloc(strlen(SERVERNAME) + 1, 0);
        strcpy(res->server, SERVERNAME);

        // Call handler
        h(req, res);
       
        // Send 
        send_response(open_socket, res);
    }

    destroy_httprequest(req);
    close(open_socket);
    return NULL;
}

void route(HTTPServer *server, enum HTTPMethod method, char *route, RouteHandler handler) {
    struct RouteTableEntry *rte = new_routetableentry(method, route, handler);
    add_route(server->rt, rte);
};

RouteHandler get_handler(HTTPServer *server, enum HTTPMethod method, char *route) {
    for (int i = 0; i < server->rt->count; i++) {
        if (server->rt->entries[i]->method == method && strcmp(server->rt->entries[i]->route, route) == 0) return server->rt->entries[i]->handler;
    }

    return NULL;
};