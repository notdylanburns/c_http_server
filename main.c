#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "http.h"
#include "server.h"

#define ROUTE(n) void n(struct HTTPRequest *req, struct HTTPResponse *res)

struct Item {
    char *key;
    char *value;
};

struct Item **items = NULL;
int count = 0;

ROUTE(get_home) {
    int size = 0;
    for (int i = 0; i < count; i++) {
        size += (strlen(items[i]->key) + strlen(items[i]->value) + 8);
    }
    if (size) {
        char response[size + 1];
        response[0] = '\0';
        for (int i = 0; i < count; i++) {
            strcat(response, items[i]->key);
            strcat(response, ": ");
            strcat(response, items[i]->value);
            strcat(response, "<br />");
        }
        set_content(res, "text/html; charset=UTF-8", size, (Bytes)response);
    } else {
        set_content(res, "text/html; charset=UTF-8", size, NULL);
    }
}

ROUTE(get_add) {
    char *k = get_urlparam(req, "key");
    char *v = get_urlparam(req, "value");
    items = realloc(items, sizeof(struct Item *) * (count + 1));
    struct Item *i = malloc(sizeof(struct Item));
    i->key = k;
    i->value = v;
    items[count++] = i;
    write_header(res, "Location", "/");
    set_status(res, "HTTP/1.1", PERMANENT_REDIRECT, "Permanent Redirect");
}

ROUTE(get_test) {
    FILE *f = fopen("html/index.html", "rb");
    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char fileContents[filesize];
    fread(fileContents, 1, filesize, f);
    printf(fileContents);

    set_content(res, "text/html; charset=UTF-8", filesize, (Bytes)fileContents);
};

ROUTE(get_image) {
    FILE *f = fopen("img/index.jpg", "rb");
    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char fileContents[filesize];
    fread(fileContents, 1, filesize, f);

    set_content(res, "image/jpg", filesize, (Bytes)fileContents);
};

int main(int argc, char **argv) {

    HTTPServer *server = new_httpserver();  

    route(server, GET, "/", &get_home);
    route(server, GET, "/add", &get_add);
    route(server, GET, "/test", &get_test);
    route(server, GET, "/image", &get_image);

    run_server(server, 8080);

    return 0;
}