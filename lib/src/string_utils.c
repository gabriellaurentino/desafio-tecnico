/**
 * @file string_utils.c
 * @author Gabriel Laurentino
 * @brief Implementação de funções auxiliares para manipulação segura de strings.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "string_utils.h"

size_t bounded_strlen(const char *str, size_t max_size) {

    if (str == NULL) {
        return 0;
    }

    size_t length = 0;

    while (length < max_size && str[length] != '\0') {
        length++;
    }

    return length;
}