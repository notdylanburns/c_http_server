#include "route.h"

struct RouteTable *new_routetable() {
    struct RouteTable *rt = malloc(sizeof(struct RouteTable));
    if (rt == NULL) return NULL;
    rt->entries = NULL;
    rt->count = 0;
    return rt;
}