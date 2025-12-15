#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zmq.h>

#if defined(_WIN32)
    #include <windows.h>
    #define PATH_SEP '\\'
    #define SLEEP(ms) Sleep(ms)
#elif defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <errno.h>
    #define PATH_SEP '/'
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

#if defined(__linux__)
    #include <sys/inotify.h>
    #include <sys/select.h>
#elif defined(__APPLE__)
    #include <sys/event.h>
    #include <fcntl.h>
#endif

#define MAX_PATH_LEN 1024
#define BUFFER_SIZE 4096
#define FILTER_EXT ".log"

typedef struct FileNode {
    char path[MAX_PATH_LEN];
    long offset;
    struct FileNode *next;
} FileNode;

FileNode *head = NULL;

void get_timestamp(char *buffer, size_t size) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", timeinfo);
}

FileNode* find_or_create_node(const char *path) {
    FileNode *current = head;
    while (current != NULL) {
        if (strcmp(current->path, path) == 0) return current;
        current = current->next;
    }
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    strncpy(node->path, path, MAX_PATH_LEN);
    node->offset = 0;
    node->next = head;
    head = node;
    return node;
}

void process_file(const char *path, void *pusher) {
    if (!strstr(path, FILTER_EXT)) return;

    FILE *fp = fopen(path, "r");
    if (!fp) return;

    FileNode *node = find_or_create_node(path);

    fseek(fp, 0, SEEK_END);
    long current_size = ftell(fp);

    // Log rotation detection
    if (current_size < node->offset) node->offset = 0;

    if (current_size > node->offset) {
        fseek(fp, node->offset, SEEK_SET);
        char line[BUFFER_SIZE];
        char time_buf[64];

        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;
            if (strlen(line) == 0) continue;

            get_timestamp(time_buf, sizeof(time_buf));

            zmq_send(pusher, path, strlen(path), ZMQ_SNDMORE);

            char formatted_msg[BUFFER_SIZE + 128];
            snprintf(formatted_msg, sizeof(formatted_msg), "%s >> %s", time_buf, line);
            zmq_send(pusher, formatted_msg, strlen(formatted_msg), 0);

            printf("Sent: %s\n", path);
        }
        node->offset = ftell(fp);
    }
    fclose(fp);
}

#if defined(_WIN32)
void scan_directory(const char *base_path, void *pusher) {
    char search_path[MAX_PATH_LEN];
    snprintf(search_path, sizeof(search_path), "%s\\*", base_path);

    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
            continue;

        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s\\%s", base_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_directory(full_path, pusher);
        } else {
            process_file(full_path, pusher);
        }
    } while (FindNextFile(hFind, &find_data) != 0);

    FindClose(hFind);
}
#else
void scan_directory(const char *base_path, void *pusher) {
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH_LEN];

    if (!(dir = opendir(base_path))) return;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) != 0) continue;

        if (S_ISDIR(statbuf.st_mode)) {
            scan_directory(path, pusher);
        } else {
            process_file(path, pusher);
        }
    }
    closedir(dir);
}
#endif

void wait_for_changes(const char *path) {
#if defined(__linux__)
    // --- LINUX INOTIFY ---
    int fd = inotify_init();
    if (fd < 0) { perror("inotify_init"); SLEEP(1000); return; }

    int wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    struct timeval timeout = {1, 0}; // 1 sec timeout (fallback)

    select(fd + 1, &fds, NULL, NULL, &timeout);

    if (FD_ISSET(fd, &fds)) {
        char buffer[1024];
        read(fd, buffer, 1024);
    }

    inotify_rm_watch(fd, wd);
    close(fd);

#elif defined(__APPLE__)
    // --- MACOS KQUEUE ---
    int kq = kqueue();
    int dir_fd = open(path, O_RDONLY);

    if (kq < 0 || dir_fd < 0) { SLEEP(1000); return; }

    struct kevent change;
    struct kevent event;

    EV_SET(&change, dir_fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_ONESHOT,
           NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB, 0, 0);

    struct timespec timeout = {1, 0};
    kevent(kq, &change, 1, &event, 1, &timeout);

    close(dir_fd);
    close(kq);

#elif defined(_WIN32)
    HANDLE hChange = FindFirstChangeNotification(
        path,
        TRUE, // TRUE = Recursive check! (Windows wins here)
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME
    );

    if (hChange == INVALID_HANDLE_VALUE) { SLEEP(1000); return; }

    WaitForSingleObject(hChange, 1000);
    FindCloseChangeNotification(hChange);

#else
    SLEEP(1000);
#endif
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <directory> <server_addr>\n", argv[0]);
        return 1;
    }

    const char *watch_dir = argv[1];
    const char *server_addr = argv[2];

    void *context = zmq_ctx_new();
    void *pusher = zmq_socket(context, ZMQ_PUB);

    int linger = 0;
    zmq_setsockopt(pusher, ZMQ_LINGER, &linger, sizeof(linger));

    if (zmq_connect(pusher, server_addr) != 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Starting Log Agent...\n");
    printf("Platform: ");
    #if defined(__linux__)
        printf("Linux (inotify)\n");
    #elif defined(__APPLE__)
        printf("macOS (kqueue)\n");
    #elif defined(_WIN32)
        printf("Windows (ReadDirectoryChanges)\n");
    #endif

    scan_directory(watch_dir, pusher);

    while (1) {
        wait_for_changes(watch_dir);
        scan_directory(watch_dir, pusher);
    }

    zmq_close(pusher);
    zmq_ctx_destroy(context);
    return 0;
}