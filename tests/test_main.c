#include "../src/log_agent_internal.h"
#include "unity.h"

extern void test_RecordCreation(void);
extern void test_RecordLookupEfficiency(void);
extern void test_LogRotationHandling(void);
extern void test_ColorSelection(void);
extern void test_EmptyLineHandling(void);

void setUp(void) {
    cleanup_records();
}

void tearDown(void) {
    cleanup_records();
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_RecordCreation);
    RUN_TEST(test_RecordLookupEfficiency);
    RUN_TEST(test_LogRotationHandling);

    RUN_TEST(test_ColorSelection);
    RUN_TEST(test_EmptyLineHandling);

    return UNITY_END();
}