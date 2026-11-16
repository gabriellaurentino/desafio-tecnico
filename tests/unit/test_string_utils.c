#include "string_utils.h"
#include "test_utils.h"

void test_bounded_strlen_null(void) {
    size_t result = bounded_strlen(NULL, 10);

    assert_equal("STRING UTILS - NULL string", 0, result);
}

void test_bounded_strlen_empty(void) {
    size_t result = bounded_strlen("", 10);

    assert_equal("STRING UTILS - Empty string", 0, result);
}

void test_bounded_strlen_normal(void) {
    size_t result = bounded_strlen("POS SYSTEM", 20);

    assert_equal("STRING UTILS - Normal string", 10, result);
}

void test_bounded_strlen_max_size(void) {
    size_t result = bounded_strlen("POS SYSTEM", 5);

    assert_equal("STRING UTILS - Maximum size reached", 5, result);
}

void test_bounded_strlen_zero_max_size(void) {
    size_t result = bounded_strlen("POS SYSTEM", 0);

    assert_equal("STRING UTILS - Zero maximum size", 0, result);
}