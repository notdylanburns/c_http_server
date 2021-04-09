#include <stdlib.h>

#include "server.h"

int main(int argc, char **argv) {

    HTTPServer *server = new_httpserver();
    run_server(server, 8080);

    return 0;
}