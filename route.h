#ifndef ROUTE_H_GUARD_
#define ROUTE_H_GUARD_

#include <stdlib.h>

#include "http.h"
#include "server.h"

struct HTTPServer;

typedef void (*RouteHandler)(struct HTTPRequest *, struct HTTPResponse *, struct HTTPServer *);

struct RouteTableEntry {
    enum HTTPMethod method;
    char *route;
    RouteHandler handler;
};

struct RouteTable {
    struct RouteTableEntry **entries;
    int count;
};

extern struct RouteTable *new_routetable();

#endif