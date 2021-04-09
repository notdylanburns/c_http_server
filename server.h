#ifndef SERVER_H_GUARD_
#define SERVER_H_GUARD_

#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "http.h"
#include "route.h"

struct HTTPServer {
    struct RouteTable *rt;
    uint8_t running;
    uint8_t error;
};

typedef struct HTTPServer HTTPServer;

extern HTTPServer *new_httpserver();
extern void route(enum HTTPMethod method, char *route, void (*handler)(struct HTTPRequest *, struct HTTPResponse *, struct HTTPServer *));
extern void run_server(HTTPServer *server, uint16_t port);
extern void stop_server(HTTPServer *server, uint8_t error);

#endif