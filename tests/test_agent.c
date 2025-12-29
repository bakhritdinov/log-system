#include <stdio.h>
#include "unity.h"
#include "../src/log_agent_internal.h"

void test_RecordCreation(void) {
    const AgentConfig config = {.tail_mode = 0};
    const FileRecord *r = get_file_record("/var/log/test.log", &config);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL_STRING("/var/log/test.log", r->path);
    TEST_ASSERT_EQUAL_INT(0, r->offset);
}

void test_RecordLookupEfficiency(void) {
    const AgentConfig config = {.tail_mode = 0};
    get_file_record("file_a.log", &config);
    get_file_record("file_a.log", &config);
    TEST_ASSERT_EQUAL_INT(1, HASH_COUNT(files));
}

void test_FreshModeTailOffset(void) {
    const AgentConfig config = {.tail_mode = 1};
    const char* filename = "fresh_test.log";

    FILE* f = fopen(filename, "w");
    fprintf(f, "existing data line 1\nexisting data line 2\n");
    const long current_pos = ftell(f);
    fclose(f);

    const FileRecord* r = get_file_record(filename, &config);

    TEST_ASSERT_EQUAL_INT(current_pos, r->offset);

    remove(filename);
}

void test_LogRotationHandling(void) {
    const AgentConfig config = {.tail_mode = 0};
    FileRecord *r = get_file_record("app.log", &config);
    r->offset = 5000;

    if (100 < r->offset) r->offset = 0;
    TEST_ASSERT_EQUAL_INT(0, r->offset);
}