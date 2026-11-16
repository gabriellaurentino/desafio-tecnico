#include <stdio.h>
#include "test_utils.h"


static int total_tests = 0;
static int passed_tests = 0;


void assert_equal(const char *test_name, int expected, int actual) {

    total_tests++;

    if (expected == actual) {
        passed_tests++;
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s | Expected: %d | Actual: %d\n", test_name, expected, actual);
    }
}

void print_test_results(void) {

    printf("\n============ TEST RESULTS ============\n");
    printf("Total: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success rate: %.2f%%\n", total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0.0);

}