#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include <sys/stat.h>
#include "log_agent_internal.h"

#if defined(_WIN32)
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <errno.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

#if defined(__linux__)
    #include <sys/inotify.h>
    #define EVENT_SIZE (sizeof(struct inotify_event))
    #define BUF_LEN (1024 * (EVENT_SIZE + 16))
#endif

#define BUFFER_SIZE 8192

FileRecord *files = NULL;

FileRecord* get_file_record(const char *path) {
    FileRecord *s = NULL;
    HASH_FIND_STR(files, path, s);
    if (!s) {
        s = (FileRecord*)calloc(1, sizeof(FileRecord));
        strncpy(s->path, path, sizeof(s->path) - 1);
        HASH_ADD_STR(files, path, s);
    }
    return s;
}

void cleanup_records() {
    FileRecord *curr, *tmp;
    HASH_ITER(hh, files, curr, tmp) {
        HASH_DEL(files, curr);
        free(curr);
    }
}

void process_file(const char *path, void *pusher) {
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    FileRecord *record = get_file_record(path);
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return; }

    long current_size = ftell(fp);
    if (current_size < record->offset) record->offset = 0;

    if (current_size > record->offset) {
        fseek(fp, record->offset, SEEK_SET);
        char line[BUFFER_SIZE];
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = 0;
            if (line[0] == '\0') continue;

            zmq_send(pusher, path, strlen(path), ZMQ_SNDMORE);
            zmq_send(pusher, line, strlen(line), 0);
        }
        record->offset = ftell(fp);
    }
    fclose(fp);
}

void scan_recursive(const char *base_path, void *pusher) {
#if defined(_WIN32)
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*", base_path);
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s\\%s", base_path, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) scan_recursive(full_path, pusher);
        else process_file(full_path, pusher);
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);
#else
    DIR *dir = opendir(base_path);
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) scan_recursive(full_path, pusher);
            else process_file(full_path, pusher);
        }
    }
    closedir(dir);
#endif
}

#ifndef TESTING
int main(int argc, char *argv[]) {
    if (argc < 3) return 1;

    const char *watch_dir = argv[1];
    void *ctx = zmq_ctx_new();
    void *pusher = zmq_socket(ctx, ZMQ_PUB);
    int hwm = 1000, linger = 0;

    zmq_setsockopt(pusher, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(pusher, ZMQ_LINGER, &linger, sizeof(linger));

    if (zmq_connect(pusher, argv[2]) != 0) return 1;

    scan_recursive(watch_dir, pusher);

#if defined(__linux__)
    int fd = inotify_init();
    if (fd < 0) return 1;
    inotify_add_watch(fd, watch_dir, IN_MODIFY);

    char buffer[BUF_LEN];
    while (1) {
        ssize_t length = read(fd, buffer, BUF_LEN);
        if (length < 0) break;
        for (char *ptr = buffer; ptr < buffer + length; ) {
            struct inotify_event *event = (struct inotify_event *)ptr;
            if ((event->mask & IN_MODIFY) && event->len) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", watch_dir, event->name);
                process_file(full_path, pusher);
            }
            ptr += EVENT_SIZE + event->len;
        }
    }
    close(fd);
#else
    while (1) {
        SLEEP(2000);
        scan_recursive(watch_dir, pusher);
    }
#endif
    cleanup_records();
    zmq_close(pusher);
    zmq_ctx_destroy(ctx);
    return 0;
}
#endif