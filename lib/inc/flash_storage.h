/**
 * @file flash_storage.h
 * @author Gabriel Laurentino
 * @brief Interface para gerenciamento de armazenamento Flash simulado.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include "errors.h"
#include <stddef.h>

/**
 * @brief Capacidade total do armazenamento simulado, em bytes.
 */
#define FLASH_STORAGE_SIZE 4096

/**
 * @brief Tamanho máximo permitido por operação, em bytes.
 */
#define FLASH_STORAGE_MAX_OPERATION_SIZE 512

/**
 * @brief Estrutura que representa o armazenamento Flash simulado.
 *
 * Armazena os dados em um vetor de bytes em memória,
 * simulando operações de leitura, escrita e apagamento.
 */
typedef struct {
    unsigned char data[FLASH_STORAGE_SIZE]; /**< Área de armazenamento. */
} FlashStorage;

/**
 * @brief Inicializa o armazenamento Flash simulado.
 *
 * Preenche toda a área de armazenamento com 0xFF,
 * representando uma memória Flash apagada.
 *
 * @param[out] storage Estrutura de armazenamento a ser inicializada.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se storage for NULL.
 */
POSStatus flash_storage_init(FlashStorage *storage);

/**
 * @brief Lê dados do armazenamento Flash.
 *
 * Copia os bytes armazenados a partir do endereço informado
 * para o buffer de destino.
 *
 * A operação deve respeitar o limite máximo de tamanho e
 * os limites da capacidade total do armazenamento.
 *
 * @param[in] storage Estrutura de armazenamento de origem.
 * @param[in] address Endereço inicial da leitura.
 * @param[out] buffer Buffer que receberá os dados lidos.
 * @param[in] length Quantidade de bytes a serem lidos.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se storage ou buffer forem NULL.
 * @return POS_ERROR_INVALID_ARGUMENT se o comprimento for inválido
 *         ou a operação exceder os limites do armazenamento.
 */
POSStatus flash_storage_read(
    const FlashStorage *storage,
    size_t address,
    unsigned char *buffer,
    size_t length
);

/**
 * @brief Grava dados no armazenamento Flash.
 *
 * Simula o comportamento de escrita de uma memória Flash,
 * permitindo apenas a transição de bits de 1 para 0.
 *
 * Para restaurar bits de 0 para 1, é necessário executar
 * uma operação de apagamento antes da gravação.
 *
 * A operação deve respeitar o limite máximo de tamanho e
 * os limites da capacidade total do armazenamento.
 *
 * @param[in,out] storage Estrutura de armazenamento a ser modificada.
 * @param[in] address Endereço inicial da gravação.
 * @param[in] data Dados que serão gravados.
 * @param[in] length Quantidade de bytes a serem gravados.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se storage ou data forem NULL.
 * @return POS_ERROR_INVALID_ARGUMENT se o comprimento for inválido,
 *         a operação exceder os limites do armazenamento ou
 *         houver tentativa de alterar bits de 0 para 1.
 */
POSStatus flash_storage_write(
    FlashStorage *storage,
    size_t address,
    const unsigned char *data,
    size_t length
);

/**
 * @brief Apaga uma região do armazenamento Flash.
 *
 * Preenche a região especificada com 0xFF, restaurando
 * todos os bits para o estado apagado.
 *
 * A operação deve respeitar o limite máximo de tamanho e
 * os limites da capacidade total do armazenamento.
 *
 * @param[in,out] storage Estrutura de armazenamento a ser modificada.
 * @param[in] address Endereço inicial da região a ser apagada.
 * @param[in] length Quantidade de bytes a serem apagados.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se storage for NULL.
 * @return POS_ERROR_INVALID_ARGUMENT se o comprimento for inválido
 *         ou a operação exceder os limites do armazenamento.
 */
POSStatus flash_storage_erase(
    FlashStorage *storage,
    size_t address,
    size_t length
);

#endif