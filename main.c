#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "server.h"

void get_home(struct HTTPRequest *req, struct HTTPResponse *res) {
    res->content_type = calloc(strlen("text/html; charset=UTF-8") + 1, 1);
    strcpy(res->content_type, "text/html; charset=UTF-8");
    res->content_length = 3;
    res->content = calloc(3, 1);
    strcpy(res->content, "Hi");
};

int main(int argc, char **argv) {

    HTTPServer *server = new_httpserver();

    route(server, GET, "/", &get_home);

    run_server(server, 8080);

    return 0;
}