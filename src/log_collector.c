#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zmq.h>

#if defined(_WIN32)
#include <windows.h>
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define C_BLUE    "\x1b[34m"
#define C_GREEN   "\x1b[32m"
#define C_YELLOW  "\x1b[33m"
#define C_RED     "\x1b[31m"
#define C_MAGENTA "\x1b[35m"
#define C_RESET   "\x1b[0m"
#define C_CYAN    "\x1b[36m"

const char *colorize_log(const char *content) {
    char temp_content[4096];
    strncpy(temp_content, content, 4096);
    for (char *p = temp_content; *p; p++) *p = tolower(*p);

    if (strstr(temp_content, "error") || strstr(temp_content, "fail") || strstr(temp_content, "exception")) {
        return C_RED;
    }
    if (strstr(temp_content, "warn") || strstr(temp_content, "timeout")) {
        return C_YELLOW;
    }
    if (strstr(temp_content, "info") || strstr(temp_content, "start") || strstr(temp_content, "connected")) {
        return C_GREEN;
    }
    if (strstr(temp_content, "debug") || strstr(temp_content, "trace")) {
        return C_CYAN;
    }

    return C_RESET;
}

void setup_console() {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    if (!GetConsoleMode(hOut, &dwMode)) {
        return;
    }

    if (SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
    }

    system("chcp 65001 > nul");
#endif
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <server_addr>\n", argv[0]);
        return 1;
    }

    const char *server_addr = argv[1];

    setup_console();
    printf("Log Collector started. Listening on port 5555...\n");

    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);

    if (zmq_bind(subscriber, server_addr) != 0) {
        perror("Bind failed");
        return 1;
    }

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    while (1) {
        char path[4096] = {0};
        char content[4096] = {0};

        int len = zmq_recv(subscriber, path, sizeof(path) - 1, 0);
        if (len == -1) continue;
        path[len] = '\0';

        const char *content_color = colorize_log(content);

        int more;
        size_t more_size = sizeof(more);
        zmq_getsockopt(subscriber, ZMQ_RCVMORE, &more, &more_size);

        if (more) {
            len = zmq_recv(subscriber, content, sizeof(content) - 1, 0);
            if (len != -1) content[len] = '\0';
        }

        printf(C_MAGENTA "%-25s" C_RESET " | %s%s" C_RESET "\n",
               path, content_color, content);
        fflush(stdout);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
