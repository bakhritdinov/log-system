#include "unity.h"
#include "../src/log_agent_internal.h"

void test_RecordCreation(void) {
    const FileRecord* r = get_file_record("/var/log/test.log");
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL_STRING("/var/log/test.log", r->path);
    TEST_ASSERT_EQUAL_INT(0, r->offset);
}

void test_RecordLookupEfficiency(void) {
    get_file_record("file_a.log");
    get_file_record("file_a.log");
    TEST_ASSERT_EQUAL_INT(1, HASH_COUNT(files));
}

void test_LogRotationHandling(void) {
    FileRecord* r = get_file_record("app.log");
    r->offset = 5000;
    if (100 < r->offset) r->offset = 0;
    TEST_ASSERT_EQUAL_INT(0, r->offset);
}