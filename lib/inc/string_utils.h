/**
 * @file string_utils.h
 * @author Gabriel Laurentino
 * @brief Funções auxiliares para manipulação segura de strings.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

/**
 * @brief Calcula o comprimento de uma string com limite máximo de leitura.
 *
 * Percorre a string até encontrar o terminador nulo ou atingir
 * o limite máximo informado.
 *
 * Se str for NULL, retorna zero.
 *
 * @param[in] str String cujo comprimento será calculado.
 * @param[in] max_size Número máximo de caracteres que podem ser examinados.
 *
 * @return Quantidade de caracteres encontrados antes do terminador nulo
 *         ou do limite máximo.
 */
size_t bounded_strlen(const char *str, size_t max_size);

#endif