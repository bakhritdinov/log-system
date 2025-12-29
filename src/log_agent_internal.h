#ifndef LOG_AGENT_INTERNAL_H
#define LOG_AGENT_INTERNAL_H

#include <uthash.h>

typedef struct {
    int tail_mode;
    int recursive;
    const char *endpoint;
} AgentConfig;

typedef struct {
    char path[1024];
    long offset;
    UT_hash_handle hh;
} FileRecord;

extern FileRecord* files;

FileRecord* get_file_record(const char* path, const AgentConfig* config);
void process_file(const char* path, void* pusher, const AgentConfig* config);
void cleanup_records();

#endif