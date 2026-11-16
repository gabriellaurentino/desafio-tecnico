/**
 * @file transaction.h
 * @author Gabriel Laurentino
 * @brief Interface pública para gerenciamento de transações POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "errors.h"


/**
 * @brief Estados possíveis de uma transação POS.
 *
 * Representa os estados do ciclo de vida de uma transação,
 * desde sua inicialização até a aprovação ou negação.
 */
typedef enum {
    POS_STATE_IDLE,       /**< Nenhuma transação em andamento. */
    POS_STATE_CARD,       /**< Aguardando leitura do cartão. */
    POS_STATE_ISO_SEND,   /**< Etapa de preparação e envio da requisição ISO8583. */
    POS_STATE_WAIT_RESP,  /**< Aguardando resposta do servidor. */
    POS_STATE_REVERSAL,   /**< Executando uma reversão após timeout. */
    POS_STATE_APPROVED,   /**< Transação aprovada. */
    POS_STATE_DECLINED    /**< Transação negada. */
} POSTransactionState;


/**
 * @brief Eventos que podem ser processados pela FSM.
 *
 * Os eventos representam ações e ocorrências durante
 * o processamento de uma transação POS.
 */
typedef enum {
    POS_EVENT_START,                /**< Inicia uma nova transação. */
    POS_EVENT_CARD_READ,            /**< Indica que o cartão foi lido. */
    POS_EVENT_REQUEST_SENT,         /**< Indica que a requisição foi enviada. */
    POS_EVENT_RESPONSE_APPROVED,    /**< Indica resposta de aprovação. */
    POS_EVENT_RESPONSE_DECLINED,    /**< Indica resposta de negação. */
    POS_EVENT_TIMEOUT,              /**< Indica timeout na comunicação. */
    POS_EVENT_REVERSAL_COMPLETED,   /**< Indica que a reversão pendente foi concluída. */
    POS_EVENT_RESET                 /**< Solicita retorno ao estado inicial. */
} POSTransactionEvent;


/**
 * @brief Contexto de execução de uma transação POS.
 *
 * Armazena o estado atual da máquina de estados e indica
 * se existe uma reversão pendente.
 */
typedef struct {
    POSTransactionState state; /**< Estado atual da transação. */
    int reversal_required;     /**< Indica se existe reversão pendente (0 ou 1). */
} POSTransaction;

/**
 * @brief Inicializa o contexto de uma transação POS.
 *
 * Define o estado inicial como POS_STATE_IDLE e inicializa
 * reversal_required com zero.
 *
 * @param[out] transaction Estrutura da transação a ser inicializada.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se transaction for NULL.
 */
POSStatus transaction_init(POSTransaction *transaction);


/**
 * @brief Processa um evento da máquina de estados da transação.
 *
 * Valida o evento de acordo com o estado atual e executa
 * a transição correspondente quando permitida.
 *
 * Em caso de timeout, a transação passa para
 * POS_STATE_REVERSAL e reversal_required passa a ser 1.
 *
 * Enquanto houver reversão pendente:
 * - Respostas de aprovação ou negação são rejeitadas.
 * - Novos eventos de timeout são rejeitados.
 * - Solicitações de reset são rejeitadas.
 *
 * A transação retorna para POS_STATE_IDLE quando o evento
 * POS_EVENT_REVERSAL_COMPLETED é recebido com uma reversão pendente.
 *
 * @param[in,out] transaction Contexto da transação a ser atualizado.
 * @param[in] event Evento a ser processado.
 *
 * @return POS_SUCCESS se o evento for processado com sucesso.
 * @return POS_ERROR_NULL_POINTER se transaction for NULL.
 * @return POS_ERROR_INVALID_ARGUMENT se o evento não for permitido
 *         para o estado atual ou violar uma regra da FSM.
 * @return POS_ERROR_INVALID_FORMAT se o estado atual for desconhecido.
 * 
 * @note A FSM controla o estado da reversão, mas não implementa
 *       a comunicação ISO8583 necessária para executar uma reversão real.
 */
POSStatus transaction_process_event(POSTransaction *transaction, POSTransactionEvent event);

#endif