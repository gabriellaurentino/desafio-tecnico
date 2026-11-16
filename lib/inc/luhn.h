/**
 * @file luhn.h
 * @author Gabriel Laurentino
 * @brief Interface para validação de números de cartão pelo algoritmo de Luhn.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef LUHN_H
#define LUHN_H

#include "errors.h"

#include <stddef.h>

/**
 * @brief Valida um número de cartão utilizando o algoritmo de Luhn.
 *
 * Verifica se o número informado:
 * - Possui entre 13 e 19 dígitos.
 * - Contém exclusivamente caracteres numéricos.
 * - Possui terminador nulo dentro do tamanho informado.
 * - Atende ao cálculo do dígito verificador do algoritmo de Luhn.
 *
 * @param[in] card_number Número do cartão a ser validado.
 * @param[in] card_number_size Tamanho máximo disponível para leitura do número,
 *                             incluindo o terminador nulo.
 *
 * @return POS_SUCCESS se o número passar na validação.
 * @return POS_ERROR_NULL_POINTER se card_number for NULL.
 * @return POS_ERROR_INVALID_LENGTH se o tamanho for zero ou o número
 *         possuir comprimento fora do intervalo permitido.
 * @return POS_ERROR_INVALID_FORMAT se não houver terminador nulo dentro
 *         do tamanho informado ou existirem caracteres não numéricos.
 * @return POS_ERROR_INVALID_PAN se o número não passar na validação de Luhn.
 */
POSStatus validate_card_number(const char card_number[], size_t card_number_size);

#endif