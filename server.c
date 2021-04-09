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

    while (server->running) {
        open_socket = accept(listener_fd, (struct sockaddr *)&address, &addrlen);
        if (open_socket < 0) {
            perror("Socket connection error");
            exit(EXIT_FAILURE);
        }

        struct HTTPRequest *req = build_httprequest(open_socket);
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
        
        close(open_socket);
    }

}