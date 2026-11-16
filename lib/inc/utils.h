/**
 * @file utils.h
 * @author Gabriel Laurentino
 * @brief Funções auxiliares para conversão de valores inteiros em bytes.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * @brief Serializa um valor uint32_t em formato Big-Endian.
 *
 * Armazena o valor em quatro bytes, começando pelo byte
 * mais significativo.
 *
 * @param[out] buffer Buffer que receberá os quatro bytes serializados.
 * @param[in] value Valor inteiro a ser convertido.
 *
 * @note O buffer deve possuir capacidade mínima de quatro bytes.
 */
void write_uint32_be(unsigned char *buffer, uint32_t value);

/**
 * @brief Desserializa quatro bytes em formato Big-Endian.
 *
 * Reconstrói um valor uint32_t a partir de uma sequência
 * de quatro bytes, considerando o byte mais significativo primeiro.
 *
 * @param[in] buffer Buffer contendo os quatro bytes serializados.
 *
 * @return Valor uint32_t reconstruído.
 *
 * @note O buffer deve possuir pelo menos quatro bytes disponíveis.
 */
uint32_t read_uint32_be(const unsigned char *buffer);

#endif