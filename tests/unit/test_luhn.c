#include "luhn.h"
#include "test_utils.h"
#include "errors.h"
#include <stddef.h>


void test_valid_card_number(void) {
    char card_number[] = "4111111111111111";
    assert_equal("LUHN Valid card number", POS_SUCCESS, validate_card_number(card_number, sizeof(card_number)));
}

void test_minimum_valid_card_number(void) {
    char card_number[] = "4000000000006";
    assert_equal("LUHN Valid card number minimum length", POS_SUCCESS, validate_card_number(card_number, sizeof(card_number)));
}

void test_maximum_valid_card_number(void) {
    char card_number[] = "4000004867895545923";
    assert_equal("LUHN Valid card number maximum length", POS_SUCCESS, validate_card_number(card_number, sizeof(card_number)));
}

void test_invalid_card_number(void) {
    char card_number[] = "4111111111111112";
    assert_equal("LUHN Invalid card number", POS_ERROR_INVALID_PAN, validate_card_number(card_number, sizeof(card_number)));
}

void test_null_card_number(void) {
    assert_equal("LUHN Null card number", POS_ERROR_NULL_POINTER, validate_card_number(NULL, 20));
}

void test_empty_card_number(void) {
    assert_equal("LUHN Empty card number", POS_ERROR_INVALID_LENGTH, validate_card_number("", 20));
}

void test_bad_format_card_number(void) {
    char card_number[] = "12345678912ABC";
    assert_equal("LUHN Bad format card number", POS_ERROR_INVALID_FORMAT, validate_card_number(card_number, sizeof(card_number)));
}

void test_card_number_too_short(void) {
    char card_number[] = "123456789";
    assert_equal("LUHN Card number too short", POS_ERROR_INVALID_LENGTH, validate_card_number(card_number, sizeof(card_number)));
}

void test_card_number_too_long(void) {
    char card_number[] = "12345678901234567891";
    assert_equal("LUHN Card number too long", POS_ERROR_INVALID_LENGTH, validate_card_number(card_number, sizeof(card_number)));
}

void test_space_in_card_number(void) {
    char card_number[] = "4111 1111 1111 1111";
    assert_equal("LUHN Space in card number", POS_ERROR_INVALID_FORMAT, validate_card_number(card_number, sizeof(card_number)));
}