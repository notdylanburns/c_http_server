#ifndef HTTP_H_GUARD_
#define HTTP_H_GUARD_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum HTTPMethod { NONE, GET, POST, PUT, DELETE };

typedef char * MimeType;

struct HTTPRequest {
    enum HTTPMethod method;
    char *route;
    char *version;
    char *host;
    char *user_agent;
    int content_length;
    MimeType content_type;
    uint8_t *content;
};

extern struct HTTPRequest *new_httprequest();
extern struct HTTPRequest *build_httprequest(int socketfd);
extern void destroy_httprequest(struct HTTPRequest *req);

struct HTTPResponse {

};

#endif
