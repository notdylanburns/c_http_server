#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "http.h"
#include "server.h"

#include "server_utils.h"

struct Item {
    char *key;
    char *value;
};

struct Item **items = NULL;
int count = 0;

STATIC(host_index_js, "scripts/index.js", "text/javascript; charset=UTF-8");
STATIC(get_test, "html/index.html", "text/html; charset=UTF-8");
STATIC(get_image, "img/index.jpg", "image/jpg");


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
};

int main(int argc, char **argv) {

    HTTPServer *server = new_httpserver();  

    route(server, GET, "/", &get_home);
    route(server, GET, "/add", &get_add);
    route(server, GET, "/test", &get_test);
    RESOURCE(server, "/image", get_image);
    RESOURCE(server, "/scripts/index.js", host_index_js);
    
    run_server(server, 8080);

    return 0;
}