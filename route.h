#ifndef ROUTE_H_GUARD_
#define ROUTE_H_GUARD_

#include "request.h"

enum HTTPMethod { GET, POST, PUT, DELETE };

typedef void (*RouteHandler)(struct HTTPRequest *req, struct HTTPResponse *res);

struct RouteTableEntry {
    enum HTTPMethod method;
    char *route;

};

struct RouteTable {
    struct RouteTableEntry **entries;
    int count;
};

#endif
