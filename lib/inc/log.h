/**
 * @file log.h
 * @author Gabriel Laurentino
 * @brief Interface para registro e consulta do histórico de transações POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef LOG_H
#define LOG_H

#include "errors.h"
#include "flash_storage.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Chave utilizada na operação XOR dos dados dos registros.
 *
 * A operação é utilizada como ofuscação dos dados armazenados,
 * não constituindo um mecanismo criptográfico seguro.
 */
#define LOG_XOR_KEY 0xAA

/**
 * @name Tamanhos dos campos serializados de um registro.
 *
 * Os valores representam os tamanhos em bytes utilizados
 * na representação persistida dos registros.
 * @{
 */
#define LOG_TIMESTAMP_SIZE 4
#define LOG_TRANSACTION_ID_SIZE 4
#define LOG_AMOUNT_SIZE 4
#define LOG_STATUS_SIZE 1
#define LOG_PAN_SIZE 19
#define LOG_RESPONSE_CODE_SIZE 2
/** @} */

/**
 * @brief Tamanho total dos campos de um registro serializado.
 */
#define LOG_RECORD_SIZE (LOG_TIMESTAMP_SIZE + LOG_TRANSACTION_ID_SIZE + LOG_AMOUNT_SIZE + LOG_STATUS_SIZE + LOG_PAN_SIZE + LOG_RESPONSE_CODE_SIZE)

/**
 * @brief Tamanho do cabeçalho associado a cada registro armazenado.
 *
 * O cabeçalho contém o tamanho do registro em dois bytes,
 * utilizando representação big-endian.
 */
#define LOG_HEADER_SIZE 2

/**
 * @brief Tamanho total ocupado por um registro no armazenamento.
 *
 * Corresponde ao tamanho dos dados serializados somado
 * ao cabeçalho utilizado pelo mecanismo de persistência.
 */
#define LOG_STORAGE_RECORD_SIZE (LOG_HEADER_SIZE + LOG_RECORD_SIZE)

/**
 * @brief Situações possíveis de uma transação registrada no histórico.
 */
typedef enum {
    LOG_TRANSACTION_APPROVED = 0, /**< Transação aprovada. */
    LOG_TRANSACTION_DECLINED = 1, /**< Transação negada. */
    LOG_TRANSACTION_TIMEOUT = 2, /**< Transação interrompida por timeout. */
    LOG_TRANSACTION_REVERSED = 3 /**< Transação revertida. */
} LogTransactionStatus;

/**
 * @brief Representação estruturada de um registro de transação.
 *
 * Contém as informações utilizadas para armazenar e recuperar
 * o histórico de transações POS.
 *
 * Os valores monetários são representados em centavos utilizando
 * uint32_t, permitindo valores de até R$ 42.949.672,95.
 *
 * O PAN armazenado no histórico é mascarado, preservando
 * os seis primeiros e os quatro últimos dígitos.
 */
typedef struct {
    uint32_t timestamp; /**< Timestamp da transação. */
    uint32_t transaction_id; /**< Identificador da transação. */
    uint32_t amount; /**< Valor da transação em centavos (máximo: R$ 42.949.672,95). */

    LogTransactionStatus status; /**< Situação da transação. */

    char pan[LOG_PAN_SIZE + 1]; /**< PAN da transação. */
    char response_code[LOG_RESPONSE_CODE_SIZE + 1]; /**< Código de resposta da transação. */
} LogRecord;

/**
 * @brief Contexto de gerenciamento do histórico de transações.
 *
 * Mantém a referência ao armazenamento Flash e os endereços
 * utilizados para operações sequenciais de leitura e escrita.
 */
typedef struct {
    FlashStorage *storage; /**< Armazenamento Flash associado ao histórico. */
    size_t write_address; /**< Endereço da próxima operação de escrita. */
    size_t read_address; /**< Endereço da próxima operação de leitura. */
} Log;

/**
 * @brief Inicializa o mecanismo de registro de transações.
 *
 * Associa o contexto de log ao armazenamento Flash informado
 * e inicializa os endereços de leitura e escrita em zero.
 *
 * Não realiza recuperação automática de registros existentes
 * no armazenamento.
 *
 * @param[out] log Contexto de gerenciamento do histórico.
 * @param[in,out] storage Armazenamento Flash utilizado para persistência.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se log ou storage forem NULL.
 */
POSStatus log_init(Log *log, FlashStorage *storage);

/**
 * @brief Grava um registro no histórico de transações.
 *
 * Valida os dados, mascara o PAN, serializa os campos,
 * aplica a operação XOR sobre os dados serializados e
 * grava o registro no armazenamento Flash.
 *
 * O endereço de escrita é avançado somente após uma
 * gravação bem-sucedida.
 *
 * @param[in,out] log Contexto de gerenciamento do histórico.
 * @param[in] record Registro de transação a ser armazenado.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se algum ponteiro obrigatório for NULL.
 * @return POS_ERROR_INVALID_ARGUMENT se os dados ou o estado forem inválidos.
 * @return POS_ERROR_INVALID_FORMAT se algum campo possuir formato inválido.
 * @return POS_ERROR_INVALID_LENGTH se o tamanho dos dados for inválido.
 * @return POS_ERROR_LOG_STORAGE_FULL se não houver espaço disponível.
 */
POSStatus log_write(Log *log, const LogRecord *record);

/**
 * @brief Recupera o próximo registro do histórico de transações.
 *
 * Realiza a leitura sequencial dos registros armazenados,
 * remove a ofuscação XOR e valida os dados recuperados.
 *
 * O endereço de leitura é avançado somente após uma
 * recuperação bem-sucedida.
 *
 * @param[in,out] log Contexto de gerenciamento do histórico.
 * @param[out] record Estrutura que receberá o registro recuperado.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se algum ponteiro obrigatório for NULL.
 * @return POS_ERROR_LOG_NO_RECORD se não houver registros disponíveis.
 * @return POS_ERROR_INVALID_ARGUMENT se os endereços ou dados forem inválidos.
 * @return POS_ERROR_INVALID_FORMAT se algum campo recuperado possuir formato inválido.
 * @return POS_ERROR_INVALID_LENGTH se o tamanho do registro for inválido.
 */
POSStatus log_read(Log *log, LogRecord *record);

/**
 * @brief Reinicia a posição de leitura do histórico.
 *
 * Define o endereço de leitura como zero, permitindo
 * percorrer novamente os registros da sessão atual.
 *
 * Não apaga os registros armazenados.
 *
 * @param[in,out] log Contexto de gerenciamento do histórico.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se log ou seu armazenamento forem NULL.
 */
POSStatus log_reset_read(Log *log);

#endif