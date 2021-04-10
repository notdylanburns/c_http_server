#include "http.h"

void bad_request(char *msg, struct HTTPRequest *req) {
    destroy_httprequest(req);
    perror(msg);
    exit(EXIT_FAILURE);
}

struct HTTPRequest *new_httprequest() {
    struct HTTPRequest *req = malloc(sizeof(struct HTTPRequest));
    if (req == NULL) return NULL;
    req->method = METHOD_NONE;
    req->route = NULL;
    req->params = NULL;
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

struct URLParam *new_urlparam(char *key, char *value) {
    struct URLParam *param = malloc(sizeof(struct URLParam));
    if (param == NULL) return NULL;

    param->key = key;
    param->value = value;
    return param;
}

void destroy_urlparam(struct URLParam *param) {
    free(param->key);
    free(param->value);
    free(param);
}

void add_param(struct HTTPRequest *req, struct URLParam *param) {
    req->paramCount++;
    req->params = realloc(req->params, req->paramCount * sizeof(struct URLParam *));
    req->params[req->paramCount - 1] = param;
}

char *readParamKey(int socketfd) {
    char nextByte;
    char *key = malloc(1);
    int keyLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == ' ') {
            return NULL;
        } else if (nextByte == '=') {
            break;
        } else {
            keyLen++;
            key = realloc(key, keyLen + 1);
            key[keyLen - 1] = nextByte;
            key[keyLen] = '\0';
        }
    }
    return key;
}

int readParamValue(int socketfd, struct URLParam *param) {
    char nextByte;
    char *value = malloc(1);
    int valueLen = 0;
    while (1) {
        read(socketfd, &nextByte, 1);
        if (nextByte == ' ') {
            param->value = value;
            return 0; //params done
        } else if (nextByte == '&') {
            param->value = value;
            return 1; //more params
        } else {
            valueLen++;
            value = realloc(value, valueLen + 1);
            value[valueLen - 1] = nextByte;
            value[valueLen] = '\0';
        }
    }
    return 0;
}

int extract_params(int socketfd, struct HTTPRequest *req) {
    char *key;
    while (1) {
        key = readParamKey(socketfd);
        if (key == NULL) {
            break;
        };
        struct URLParam *p = new_urlparam(key, NULL);
        int moreParams = readParamValue(socketfd, p);
        add_param(req, p);
        
        if (!moreParams) break; 
    }
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
        } else if (nextByte == '?') {
            extract_params(socketfd, req);
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
        if (nextByte == '\r') {
            read(socketfd, &nextByte, 1); //discard newline
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

struct URLParam *get_urlparam(struct HTTPRequest *req, char *key) {
    for (int i = 0; i < req->paramCount; i++) {
        if (strcmp(req->params[i]->key, key) == 0) return req->params[i];
    }
    return NULL;
}

void destroy_httprequest(struct HTTPRequest *req) {
    free(req->route);
    free(req->version);
    free(req->host);
    free(req->user_agent);
    free(req->content_type);
    free(req->content);
    for (int i = 0; i < req->paramCount; i++) destroy_urlparam(req->params[i]);
    free(req->params);
    free(req);
    req = NULL;
}

struct HTTPResponse *new_httpresponse() {
    struct HTTPResponse *res = malloc(sizeof(struct HTTPResponse));
    if (res == NULL) return NULL;
    res->version = NULL;
    res->status = STATUS_NONE;
    res->status_msg = NULL;
    res->date = NULL;
    res->server = NULL;
    res->content_type = NULL;
    res->content_length = 0;
    res->content = NULL;
    return res;
}

char *build_httpresponse(struct HTTPResponse *res) {
    int headerLen;
    char *header;
    if (res->content_length) {
        headerLen = (strlen(res->version) + 1) /* version + ' ' */ + 
                        4 /* status + ' ' */ + 
                        (strlen(res->status_msg) + 2) /* status_msg + \r\n */ +
                        38 /* Date: day dd mm yyyy hh:mm:ss UTC\r\n */ +
                        10 + strlen(res->server) /* Server: server\r\n */ +
                        14 + strlen(res->content_type) /* Content-Type: content_type */ +
                        18 + floor(log10(abs(res->content_length))) + 1 /* Content-Length: content_length\r\n */ +
                        2 /* \r\n */;
        header = malloc(headerLen + 1);
        sprintf(header, "%s %03d %s\r\nDate: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", res->version, res->status, res->status_msg, res->date, res->server, res->content_type, res->content_length);
    } else {
        headerLen = (strlen(res->version) + 1) /* version + ' ' */ + 
                        4 /* status + ' ' */ + 
                        (strlen(res->status_msg) + 2) /* status_msg + \r\n */ +
                        38 /* Date: day dd mm yyyy hh:mm:ss UTC\r\n */ +
                        10 + strlen(res->server) /* Server: server\r\n */ +
                        2 /* \r\n */;
        header = malloc(headerLen + 1);
        sprintf(header, "%s %03d %s\r\nDate: %s\r\nServer: %s\r\n\r\n", res->version, res->status, res->status_msg, res->date, res->server);
    
    }
    
    return header;

}

void destroy_httpresponse(struct HTTPResponse *res) {
    free(res->version);
    free(res->status_msg);
    free(res->date);
    free(res->server);
    free(res->content_type);
    free(res->content);
    free(res);
}