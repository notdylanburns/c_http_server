#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "server.h"

void get_home(struct HTTPRequest *req, struct HTTPResponse *res) {
    set_content(res, "text/html; charset=UTF-8", 5, "test");
};

int main(int argc, char **argv) {

    HTTPServer *server = new_httpserver();

    route(server, GET, "/", &get_home);

    run_server(server, 8080);

    return 0;
}