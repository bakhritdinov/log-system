#include "../src/log_collector_internal.h"
#include "unity.h"

void test_ColorSelection(void) {
    TEST_ASSERT_EQUAL_STRING(C_RED, get_color("system error: disk failure"));
    TEST_ASSERT_EQUAL_STRING(C_GREEN, get_color("user info: login success"));
    TEST_ASSERT_EQUAL_STRING(C_YELLOW, get_color("warning: low memory"));
    TEST_ASSERT_EQUAL_STRING(C_RESET, get_color("just a message"));
}

void test_EmptyLineHandling(void) {
    TEST_ASSERT_EQUAL_STRING(C_RESET, get_color(""));
}