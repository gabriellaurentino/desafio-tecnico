/**
 * @file pos_app.h
 * @author Gabriel Laurentino
 * @brief Interface de integração e gerenciamento da aplicação POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 *
 * @details
 * Integra os componentes da aplicação POS, coordenando o ciclo de vida
 * das transações, leitura e validação do cartão, construção de mensagens
 * ISO8583, processamento de respostas, tratamento de timeout e reversões.
 *
 * Também integra o armazenamento Flash virtual e o registro de eventos
 * da transação.
 *
 * Permite configurar uma função externa para obtenção de data e hora,
 * facilitando testes determinísticos e a portabilidade entre plataformas.
 */

#ifndef POS_APP_H
#define POS_APP_H

#include <stddef.h>
#include <stdint.h>

#include "transaction.h"
#include "iso8583.h"
#include "flash_storage.h"
#include "log.h"


/**
 * @brief Representa uma data e hora.
 *
 * Estrutura utilizada para fornecer informações temporais à aplicação
 * por meio de um callback configurável.
 *
 * O ano aceito pela aplicação deve estar entre 2000 e 2099.
 */
typedef struct {
    int year;       /**< Ano com quatro dígitos. */
    int month;      /**< Mês (1 a 12). */
    int day;        /**< Dia do mês. */
    int hour;       /**< Hora (0 a 23). */
    int minute;     /**< Minuto (0 a 59). */
    int second;     /**< Segundo (0 a 59). */
} POSDateTime;


/**
 * @brief Callback responsável por fornecer data e hora.
 *
 * Permite que a aplicação obtenha informações temporais sem depender
 * diretamente de uma implementação específica de relógio ou sistema
 * operacional.
 *
 * @param[out] datetime Estrutura que receberá a data e hora.
 * @param[in] context Contexto definido pelo chamador.
 *
 * @return POS_SUCCESS quando a data e hora forem obtidas.
 * @return POS_ERROR_NULL_POINTER quando datetime for nulo.
 * @return Outro código POSStatus definido pela implementação do callback.
 */
typedef POSStatus (*POSClockCallback)(POSDateTime *datetime, void *context);


/**
 * @brief Dados necessários para construir uma requisição financeira.
 *
 * Agrupa os campos utilizados na montagem da mensagem ISO8583 MTI 0200.
 *
 * O valor monetário é informado em centavos e deve respeitar o limite
 * de 12 dígitos do campo ISO8583 correspondente.
 */
typedef struct {
    char processing_code[ISO8583_SIZE_PROCESSING_CODE + 1];
    /**< Código de processamento ISO8583. */

    uint64_t amount;
    /**< Valor da transação em centavos. */

    char terminal_id[ISO8583_SIZE_TERMINAL_ID + 1];
    /**< Identificador do terminal POS. */

    char currency_code[ISO8583_SIZE_CURRENCY_CODE + 1];
    /**< Código numérico da moeda. */
} POSTransactionRequest;


/**
 * @brief Contexto principal da aplicação POS.
 *
 * Mantém o estado da transação atual e as dependências necessárias
 * para executar o fluxo de processamento.
 *
 * Integra a máquina de estados, armazenamento Flash, registro de logs,
 * dados do cartão, informações monetárias e mecanismo de obtenção
 * de data e hora.
 */
typedef struct {
    POSTransaction transaction;
    /**< Máquina de estados da transação atual. */

    FlashStorage *storage;
    /**< Referência ao armazenamento Flash virtual. */

    Log log;
    /**< Instância responsável pelo registro de eventos. */

    char pan[ISO8583_SIZE_PAN + 1];
    /**< PAN do cartão utilizado na transação. */

    char amount[ISO8583_SIZE_AMOUNT + 1];
    /**< Valor da transação armazenado como texto decimal. */

    uint32_t stan_counter;
    /**< Contador utilizado para geração do STAN. */

    POSClockCallback clock_callback;
    /**< Callback responsável por fornecer data e hora. */

    void *clock_context;
    /**< Contexto adicional encaminhado ao callback de relógio. */
} POSApp;


/**
 * @brief Configura a função responsável por fornecer data e hora.
 *
 * Armazena o callback e seu contexto para utilização durante a
 * construção de mensagens e geração de registros de log.
 *
 * @param[in,out] app Instância da aplicação POS.
 * @param[in] callback Função utilizada para obter data e hora.
 * @param[in] context Contexto opcional encaminhado ao callback.
 *
 * @return POS_SUCCESS quando a configuração for realizada.
 * @return POS_ERROR_NULL_POINTER quando app ou callback forem nulos.
 */
POSStatus pos_app_set_clock(POSApp *app, POSClockCallback callback, void *context);


/**
 * @brief Inicializa a aplicação POS.
 *
 * Inicializa o contexto da aplicação, a máquina de estados, o
 * armazenamento Flash e o sistema de registro de logs.
 *
 * @param[out] app Instância da aplicação POS.
 * @param[in,out] storage Armazenamento Flash virtual utilizado pela aplicação.
 *
 * @return POS_SUCCESS quando a inicialização for concluída.
 * @return POS_ERROR_NULL_POINTER quando app ou storage forem nulos.
 * @return Outro código POSStatus propagado por flash_storage_init()
 *         ou log_init().
 */
POSStatus pos_app_init(POSApp *app, FlashStorage *storage);


/**
 * @brief Inicia uma nova transação POS.
 *
 * Solicita à máquina de estados o evento de início da transação,
 * preparando a aplicação para receber os dados do cartão.
 *
 * @param[in,out] app Instância da aplicação POS.
 *
 * @return POS_SUCCESS quando a transação for iniciada.
 * @return POS_ERROR_NULL_POINTER quando app for nulo.
 * @return Outro código POSStatus propagado por transaction_process_event().
 */
POSStatus pos_app_start_transaction(POSApp *app);


/**
 * @brief Processa a leitura do cartão.
 *
 * Valida o PAN utilizando o algoritmo de Luhn, armazena o número
 * do cartão no contexto da aplicação e atualiza a máquina de estados.
 *
 * @param[in,out] app Instância da aplicação POS.
 * @param[in] pan Número do cartão (PAN).
 *
 * @return POS_SUCCESS quando o cartão for validado e processado.
 * @return POS_ERROR_NULL_POINTER quando app ou pan forem nulos.
 * @return POS_ERROR_APP_INVALID_STATE quando a transação não estiver
 *         no estado POS_STATE_CARD.
 * @return POS_ERROR_INVALID_LENGTH quando o PAN estiver vazio ou
 *         exceder o tamanho máximo permitido.
 * @return Outro código POSStatus propagado por validate_card_number()
 *         ou transaction_process_event().
 */
POSStatus pos_app_card_read(POSApp *app, const char *pan);


/**
 * @brief Constrói uma requisição financeira ISO8583.
 *
 * Utiliza os dados do cartão, os parâmetros da requisição e as
 * informações temporais para construir uma mensagem ISO8583 MTI 0200.
 *
 * Gera o STAN da transação e atualiza o contexto da aplicação após
 * a construção bem-sucedida da mensagem e a transição da máquina
 * de estados.
 *
 * @param[in,out] app Instância da aplicação POS.
 * @param[in] request Dados utilizados na construção da requisição.
 * @param[out] buffer Buffer que receberá a mensagem ISO8583.
 *
 * @return POS_SUCCESS quando a mensagem for construída.
 * @return POS_ERROR_NULL_POINTER quando algum parâmetro obrigatório for nulo.
 * @return POS_ERROR_APP_INVALID_STATE quando a transação não estiver
 *         no estado POS_STATE_ISO_SEND.
 * @return POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED quando não houver
 *         PAN armazenado.
 * @return POS_ERROR_INVALID_ARGUMENT quando o valor exceder 12 dígitos
 *         ou quando não houver callback de relógio configurado.
 * @return POS_ERROR_APP_INVALID_DATETIME quando a data ou hora forem inválidas.
 * @return POS_ERROR_APP_INVALID_TRANSMISSION_DATE quando não for possível
 *         formatar o campo de data e hora de transmissão.
 * @return POS_ERROR_APP_INVALID_LOCAL_TIME quando não for possível
 *         formatar o campo de hora local.
 * @return POS_ERROR_APP_INVALID_STAN quando não for possível formatar o STAN.
 * @return Outro código POSStatus propagado pelo callback de relógio,
 *         iso8583_build() ou transaction_process_event().
 */
POSStatus pos_app_build_request(POSApp *app, const POSTransactionRequest *request, ISO8583Buffer *buffer);


/**
 * @brief Processa a resposta recebida do processamento financeiro.
 *
 * Interpreta a mensagem ISO8583 recebida, valida o MTI, o STAN e o
 * código de resposta, atualiza a máquina de estados e registra o
 * resultado da transação.
 *
 * O código de resposta "00" representa aprovação. Os demais códigos
 * numéricos representam negação.
 *
 * @param[in,out] app Instância da aplicação POS.
 * @param[in] data Dados recebidos contendo a mensagem ISO8583.
 * @param[in] data_size Tamanho da mensagem recebida em bytes.
 *
 * @return POS_SUCCESS quando a resposta for processada e registrada.
 * @return POS_ERROR_NULL_POINTER quando app ou data forem nulos.
 * @return POS_ERROR_APP_INVALID_STATE quando a transação não estiver
 *         aguardando resposta ou possuir reversão pendente.
 * @return POS_ERROR_INVALID_MTI quando o MTI recebido for diferente de 0210.
 * @return POS_ERROR_INVALID_FIELD quando o STAN não corresponder ao
 *         da transação atual.
 * @return POS_ERROR_INVALID_FIELD_LENGTH quando o código de resposta
 *         possuir tamanho inválido.
 * @return POS_ERROR_INVALID_FIELD_FORMAT quando o código de resposta
 *         contiver caracteres não numéricos.
 * @return POS_ERROR_APP_INVALID_STAN quando não for possível formatar
 *         o STAN esperado.
 * @return Outro código POSStatus propagado por iso8583_parse(),
 *         get_log_timestamp(), transaction_process_event() ou log_write().
 */
POSStatus pos_app_process_response(POSApp *app, const unsigned char *data, size_t data_size);


/**
 * @brief Processa o timeout de uma transação.
 *
 * Trata o esgotamento do tempo de espera por uma resposta financeira,
 * solicitando à máquina de estados a transição para POS_STATE_REVERSAL.
 *
 * O evento de timeout também é registrado no log da aplicação.
 *
 * @param[in,out] app Instância da aplicação POS.
 *
 * @return POS_SUCCESS quando o timeout for processado e registrado.
 * @return POS_ERROR_NULL_POINTER quando app for nulo.
 * @return POS_ERROR_APP_INVALID_STATE quando a transação não estiver
 *         em POS_STATE_WAIT_RESP ou já possuir reversão pendente.
 * @return POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED quando não houver
 *         PAN armazenado para o registro.
 * @return POS_ERROR_INVALID_ARGUMENT quando o valor da transação
 *         não estiver disponível ou não houver callback de relógio.
 * @return POS_ERROR_APP_INVALID_DATETIME quando a data ou hora forem inválidas.
 * @return Outro código POSStatus propagado por transaction_process_event(),
 *         get_log_timestamp() ou log_write().
 */
POSStatus pos_app_process_timeout(POSApp *app);


/**
 * @brief Processa a conclusão de uma reversão.
 *
 * Informa à máquina de estados que o processo de reversão foi concluído,
 * permitindo encerrar o ciclo da transação e retornar ao estado inicial.
 *
 * O evento de conclusão também é registrado no log da aplicação.
 *
 * @param[in,out] app Instância da aplicação POS.
 *
 * @return POS_SUCCESS quando a reversão for concluída e registrada.
 * @return POS_ERROR_NULL_POINTER quando app for nulo.
 * @return POS_ERROR_APP_INVALID_STATE quando não existir uma reversão
 *         pendente ou o estado atual não permitir sua conclusão.
 * @return POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED quando não houver
 *         PAN armazenado para o registro.
 * @return POS_ERROR_INVALID_ARGUMENT quando o valor da transação
 *         não estiver disponível ou não houver callback de relógio.
 * @return POS_ERROR_APP_INVALID_DATETIME quando a data ou hora forem inválidas.
 * @return Outro código POSStatus propagado por transaction_process_event(),
 *         get_log_timestamp() ou log_write().
 */
POSStatus pos_app_process_reversal_completed(POSApp *app);


/**
 * @brief Reinicializa o estado da transação atual.
 *
 * Solicita à máquina de estados o retorno ao estado inicial.
 * Após a transição bem-sucedida, limpa os dados locais do PAN e
 * do valor monetário da transação.
 *
 * A reinicialização é permitida somente nos estados CARD, ISO_SEND,
 * APPROVED e DECLINED.
 *
 * @param[in,out] app Instância da aplicação POS.
 *
 * @return POS_SUCCESS quando a transação for reinicializada.
 * @return POS_ERROR_NULL_POINTER quando app for nulo.
 * @return POS_ERROR_APP_INVALID_STATE quando o estado atual não permitir
 *         a reinicialização.
 * @return Outro código POSStatus propagado por transaction_process_event().
 */
POSStatus pos_app_reset_transaction(POSApp *app);

#endif