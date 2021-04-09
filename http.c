#include "http.h"

void bad_request(char *msg, struct HTTPRequest *req) {
    destroy_httprequest(req);
    perror(msg);
    exit(EXIT_FAILURE);
}

struct HTTPRequest *new_httprequest() {
    struct HTTPRequest *req = malloc(sizeof(struct HTTPRequest));
    if (req == NULL) return NULL;
    req->method = NONE;
    req->route = NULL;
    req->version = NULL;
    req->host = NULL;
    req->user_agent = NULL;
    req->content_length = 0;
    req->content_type = NULL;
    req->content = NULL;
    return req;
}

int extract_method(int socketfd, struct HTTPRequest *req) {
    char nextByte;
    char *method = malloc(1);
    int methodLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == ' ') {
            break;
        } else {
            methodLen++;
            method = realloc(method, methodLen + 1);
            method[methodLen - 1] = nextByte;
            method[methodLen] = '\0';
        }
    }

    if (strcmp(method, "GET") == 0) {
        req->method = GET;
    } else if (strcmp(method, "POST") == 0) {
        req->method = POST;
    } else if (strcmp(method, "PUT") == 0) {
        req->method = PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        req->method = DELETE;
    } else { 
        free(method);
        return 1;
    }

    free(method);

    return 0;
}

int extract_route(int socketfd, struct HTTPRequest *req) {
    char nextByte;
    char *route = malloc(1);
    int routeLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == ' ') {
            break;
        } else {
            routeLen++;
            route = realloc(route, routeLen + 1);
            route[routeLen - 1] = nextByte;
            route[routeLen] = '\0';
        }
    }

    req->route = route;
    return 0;

}

int extract_version(int socketfd, struct HTTPRequest *req) {
    char nextByte;
    char *version = malloc(1);
    int versionLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == '\r') {
            read(socketfd, &nextByte, 1); //discard newline
            break;
        } else {
            versionLen++;
            version = realloc(version, versionLen + 1);
            version[versionLen - 1] = nextByte;
            version[versionLen] = '\0';
        }
    }

    req->version = version;
    return 0;
}

char *readKey(int socketfd) {
    char nextByte;
    char *key = malloc(1);
    int keyLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == ' ') {
            break;
        } else {
            keyLen++;
            key = realloc(key, keyLen + 1);
            key[keyLen - 1] = nextByte;
            key[keyLen] = '\0';
            if (strcmp(key, "\r\n") == 0) return NULL;
        }
    }

    return key;
}

char *readValue(int socketfd) {
    char nextByte;
    char *value = malloc(1);
    int valueLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == '\n') {
            break;
        } else {
            valueLen++;
            value = realloc(value, valueLen + 1);
            value[valueLen - 1] = nextByte;
            value[valueLen] = '\0';
        }
    }

    return value;
}

struct HTTPRequest *build_httprequest(int socketfd) {
    struct HTTPRequest *req = new_httprequest();
    if (req == NULL) return NULL;

    if (extract_method(socketfd, req) > 0) {
        bad_request("Failed to extract method from HTTP request", req);
    } else if (extract_route(socketfd, req) > 0) {
        bad_request("Failed to extract route from HTTP request", req);
    } else if (extract_version(socketfd, req) > 0) {
        bad_request("Failed to extract version from HTTP request", req);
    } 

    while (1) {
        char *key = readKey(socketfd);
        if (key == NULL) {
            //End of headers
            break;
        }

        if (strcmp(key, "Host:") == 0) {
            req->host = readValue(socketfd);
        } else if (strcmp(key, "User-Agent:") == 0) {
            req->user_agent = readValue(socketfd);
        } else if (strcmp(key, "Content-Length:") == 0) {
            req->content_length = atoi(readValue(socketfd));
        } else if (strcmp(key, "Content-Type:") == 0) {
            req->content_type = readValue(socketfd);
        } else {
            readValue(socketfd);
        }
    }

    if (req->content_length == 0) {
        return req;
    } else {
        /*char continue100[13] = "100 Continue";
        send(socketfd, continue100, 13, 0);*/

        req->content = calloc(req->content_length + 1, 1);
        read(socketfd, req->content, req->content_length);

    }

    return req;

}

void destroy_httprequest(struct HTTPRequest *req) {
    free(req->route);
    free(req->version);
    free(req->host);
    free(req->user_agent);
    free(req->content_type);
    free(req->content);
    free(req);
    req = NULL;
}