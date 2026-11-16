/**
 * @file luhn.c
 * @author Gabriel Laurentino
 * @brief Implementação da validação de números de cartão pelo algoritmo de Luhn.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "luhn.h"

#include <ctype.h>

#include "string_utils.h"


POSStatus validate_card_number(const char card_number[], size_t card_number_size) {

    if (card_number == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (card_number_size == 0) {
        return POS_ERROR_INVALID_LENGTH;
    }

    int value = 0;
    size_t len = bounded_strlen(card_number, card_number_size);

    if (len == card_number_size) {
        return POS_ERROR_INVALID_FORMAT;
    }


    if (len < 13 || len > 19) {
        return POS_ERROR_INVALID_LENGTH;
    }

    for (size_t i = len; i > 0; i--) {

        size_t index = i - 1;
        
        if (!isdigit((unsigned char) card_number[index])) {
            return POS_ERROR_INVALID_FORMAT;
        }

        int digit = card_number[index] - '0';
        
        if ((len - 1 - index) % 2 == 1) {
            digit *= 2;
            if (digit > 9) {
                digit -= 9;
            }
            
        }
        value += digit;
    }

    if ((value % 10) == 0) {
        return POS_SUCCESS;
    }

    return POS_ERROR_INVALID_PAN;

}