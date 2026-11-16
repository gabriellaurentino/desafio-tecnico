/**
 * @file utils.c
 * @author Gabriel Laurentino
 * @brief Implementação de funções auxiliares para conversão de valores inteiros em bytes.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "utils.h"

void write_uint32_be(unsigned char *buffer, uint32_t value) {

    buffer[0] = (unsigned char)((value >> 24) & 0xFF);
    buffer[1] = (unsigned char)((value >> 16) & 0xFF);
    buffer[2] = (unsigned char)((value >> 8) & 0xFF);
    buffer[3] = (unsigned char)(value & 0xFF);

}

uint32_t read_uint32_be(const unsigned char *buffer) {
    return ((uint32_t)buffer[0] << 24) |
           ((uint32_t)buffer[1] << 16) |
           ((uint32_t)buffer[2] << 8) |
           (uint32_t)buffer[3];
}