#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "log_collector_internal.h"

#if defined(_WIN32)
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

const char* get_color(const char* line) {
    if (strstr(line, "error") || strstr(line, "critical")) return C_RED;
    if (strstr(line, "warning")) return C_YELLOW;
    if (strstr(line, "info")) return C_GREEN;
    if (strstr(line, "debug")) return C_CYAN;
    return C_RESET;
}

void setup_console() {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

#ifndef TESTING
int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    setup_console();
    void* ctx = zmq_ctx_new();
    void* sub = zmq_socket(ctx, ZMQ_SUB);

    if (zmq_bind(sub, argv[1]) != 0) return 1;
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);

    while (1) {
        char path[1024] = {0}, line[8192] = {0};
        int len = zmq_recv(sub, path, sizeof(path) - 1, 0);
        if (len == -1) continue;
        path[len] = 0;

        int more;
        size_t m_sz = sizeof(more);
        zmq_getsockopt(sub, ZMQ_RCVMORE, &more, &m_sz);

        if (more) {
            len = zmq_recv(sub, line, sizeof(line) - 1, 0);
            if (len != -1) line[len] = 0;
        }

        printf("%s%-25s%s | %s%s%s\n", C_MAGENTA, path, C_RESET, get_color(line), line, C_RESET);
        fflush(stdout);
    }

    zmq_close(sub);
    zmq_ctx_destroy(ctx);
    return 0;
}
#endif