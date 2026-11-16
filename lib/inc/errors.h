/**
 * @file errors.h
 * @author Gabriel Laurentino
 * @brief Definição dos códigos de retorno da biblioteca e aplicação POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef ERRORS_H
#define ERRORS_H


/**
 * @brief Códigos de retorno das operações da biblioteca e aplicação POS.
 *
 * Define um padrão único para indicar sucesso ou falha nas operações.
 *
 * O valor POS_SUCCESS (0) representa uma operação concluída com sucesso.
 * Os valores negativos representam erros, agrupados por categoria:
 *
 * - -1 a -99: Erros gerais.
 * - -100 a -199: Erros de validação.
 * - -200 a -299: Erros relacionados ao protocolo ISO8583.
 * - -300 a -399: Erros relacionados ao registro de transações (LOG).
 * - -400 a -499: Erros relacionados à aplicação POS.
 *
 * Os códigos são utilizados como retorno das funções públicas
 * e permitem que a aplicação identifique e trate cada falha.
 */
typedef enum {
    POS_SUCCESS = 0, /**< Operação concluída com sucesso. */

    /* Erros gerais */
    POS_ERROR_NULL_POINTER = -1,       /**< Ponteiro obrigatório não informado. */
    POS_ERROR_INVALID_ARGUMENT = -2,  /**< Argumento inválido. */
    POS_ERROR_BUFFER_OVERFLOW = -3,    /**< Capacidade do buffer excedida. */
    POS_ERROR_TIMEOUT = -4,            /**< Operação excedeu o tempo limite. */

    /* Erros de validação */
    POS_ERROR_INVALID_LENGTH = -100,   /**< Comprimento de dado inválido. */
    POS_ERROR_INVALID_FORMAT = -101,   /**< Formato de dado inválido. */
    POS_ERROR_INVALID_PAN = -102,      /**< Número do cartão (PAN) inválido. */

    /* Erros ISO8583 */
    POS_ERROR_INVALID_MTI = -200,          /**< MTI inválido. */
    POS_ERROR_INVALID_BITMAP = -201,       /**< Bitmap ISO8583 inválido. */
    POS_ERROR_INVALID_FIELD = -202,        /**< Campo ISO8583 inválido ou não suportado. */
    POS_ERROR_INVALID_FIELD_LENGTH = -203, /**< Comprimento de campo ISO8583 inválido. */
    POS_ERROR_INVALID_FIELD_FORMAT = -204, /**< Formato de campo ISO8583 inválido. */

    /* Erros LOG */
    POS_ERROR_LOG_INVALID_EVENT = -300, /**< Evento de log inválido. */
    POS_ERROR_LOG_LENGTH_EXCEEDED = -301, /**< Limite de registros ou tamanho excedido. */
    POS_ERROR_LOG_STORAGE_FULL = -302, /**< Armazenamento sem espaço disponível. */
    POS_ERROR_LOG_NO_RECORD = -303, /**< Nenhum registro encontrado. */

    /* Erros da aplicação */
    POS_ERROR_APP_INVALID_STATE = -400, /**< Estado inválido para a operação solicitada. */
    POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED = -401, /**< Número do cartão não informado. */
    POS_ERROR_APP_INVALID_DATETIME = -402, /**< Data e hora inválidas. */
    POS_ERROR_APP_INVALID_TRANSMISSION_DATE = -403, /**< Data de transmissão inválida. */
    POS_ERROR_APP_INVALID_LOCAL_TIME = -404, /**< Horário local inválido. */
    POS_ERROR_APP_INVALID_STAN = -405 /**< STAN inválido. */

} POSStatus;

#endif