#ifndef LOG_AGENT_INTERNAL_H
#define LOG_AGENT_INTERNAL_H

#include <uthash.h>

typedef struct {
    char path[1024];
    long offset;
    UT_hash_handle hh;
} FileRecord;

extern FileRecord* files;

FileRecord* get_file_record(const char* path);

void cleanup_records();
void process_file(const char* path, void* pusher);

#endif