#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pos_app.h"

#define INPUT_SIZE 64

#define PROCESSING_CODE "000000"
#define TERMINAL_ID     "TERM0001"
#define CURRENCY_CODE   "986"
#define MAX_ISO_AMOUNT 999999999999ULL

static int read_line(const char *message, char *buffer, size_t size) {

    if (buffer == NULL || size == 0) {
        return 0;
    }

    printf("%s", message);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return 1;
}


static POSStatus get_current_datetime(POSDateTime *datetime, void *context) {

    time_t current_time;
    struct tm *local_time;

    (void)context;

    if (datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    current_time = time(NULL);
    local_time = localtime(&current_time);

    if (local_time == NULL) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    datetime->year = local_time->tm_year + 1900;
    datetime->month = local_time->tm_mon + 1;
    datetime->day = local_time->tm_mday;
    datetime->hour = local_time->tm_hour;
    datetime->minute = local_time->tm_min;
    datetime->second = local_time->tm_sec;

    return POS_SUCCESS;
}


static int parse_amount(const char *input, uint64_t *amount) {

    if (input == NULL || amount == NULL || input[0] == '\0') {
        return 0;
    }

    uint64_t integer_part = 0;
    uint64_t decimal_part = 0;

    int integer_digits = 0;
    int decimal_digits = 0;
    int decimal_separator = 0;

    for (size_t i = 0; input[i] != '\0'; i++) {

        char c = input[i];

        if (c == ',' || c == '.') {

            if (decimal_separator || integer_digits == 0) {
                return 0;
            }

            decimal_separator = 1;
            continue;
        }

        if (c < '0' || c > '9') {
            return 0;
        }

        int digit = c - '0';

        if (!decimal_separator) {

            if (integer_part > 9999999999ULL / 10 ||
                (integer_part == 9999999999ULL / 10 &&
                 (uint64_t)digit > 9999999999ULL % 10)) {
                return 0;
            }

            integer_part = integer_part * 10 + (uint64_t)digit;
            integer_digits++;

        } else {

            if (decimal_digits >= 2) {
                return 0;
            }

            decimal_part = decimal_part * 10 + (uint64_t)digit;
            decimal_digits++;
        }
    }

    if (integer_digits == 0 ||
        (decimal_separator && decimal_digits == 0)) {
        return 0;
    }

    if (decimal_digits == 1) {
        decimal_part *= 10;
    }

    *amount = integer_part * 100 + decimal_part;

    return *amount <= MAX_ISO_AMOUNT;
}


static void show_menu(void) {

    printf("\n");
    printf("================================\n");
    printf("          POS TERMINAL\n");
    printf("================================\n");
    printf("\n");
    printf("1 - Vender\n");
    printf("2 - Historico de transacoes\n");
    printf("3 - Sair\n");
    printf("\n");
}


static POSStatus simulate_authorization(const ISO8583Buffer *request_buffer, ISO8583Buffer *response_buffer, const char *response_code) {
    ISO8583Message message;
    POSStatus status;

    if (request_buffer == NULL || response_buffer == NULL || response_code == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    status = iso8583_parse(request_buffer->data, request_buffer->length, &message);

    if (status != POS_SUCCESS) {
        return status;
    }

    memcpy(message.mti, "0210", 4);
    memcpy(message.response_code, response_code, 2);

    return iso8583_build(&message, response_buffer);
}

static const char *get_simulated_response_code(uint64_t amount) {

    if (amount == 50000) {
        return "05"; /* Transação negada */
    }

    return "00"; /* Transação aprovada */
}

static void process_sale(POSApp *app) {

    char pan[ISO8583_SIZE_PAN + 1];
    char input_amount[INPUT_SIZE];
    uint64_t amount;

    POSTransactionRequest request;
    ISO8583Buffer request_buffer;
    ISO8583Buffer response_buffer;

    POSStatus status;

    printf("\n========== NOVA VENDA ==========\n\n");

    if (!read_line("Informe o PAN: ", pan, sizeof(pan))) {
        return;
    }

    if (!read_line("Informe o valor (ex: 10,50): ",
                   input_amount, sizeof(input_amount))) {
        return;
    }

    if (!parse_amount(input_amount, &amount)) {
        printf("\nValor informado invalido.\n");
        return;
    }

    /*
     * Configura os dados da solicitacao.
     */
    memset(&request, 0, sizeof(request));

    strcpy(request.processing_code, PROCESSING_CODE);
    request.amount = amount;
    strcpy(request.terminal_id, TERMINAL_ID);
    strcpy(request.currency_code, CURRENCY_CODE);

    /*
     * Inicia o fluxo da transacao.
     */
    status = pos_app_start_transaction(app);

    if (status != POS_SUCCESS) {
        printf("Erro ao iniciar transacao: %d\n", status);
        return;
    }

    /*
     * Simula a leitura do cartao.
     */
    status = pos_app_card_read(app, pan);

    if (status != POS_SUCCESS) {
        printf("Erro na leitura do cartao: %d\n", status);

        POSStatus reset_status = pos_app_reset_transaction(app);
        if (reset_status != POS_SUCCESS) {
            printf("Erro ao resetar transacao: %d\n", reset_status);
        }
        return;
    }

    /*
     * Constroi a mensagem ISO8583 0200.
     */
    status = pos_app_build_request(app, &request, &request_buffer);

    if (status != POS_SUCCESS) {
        printf("Erro ao construir requisicao: %d\n", status);
        return;
    }

    printf("\nSolicitacao ISO8583 0200 construida.\n");
    printf("PAN: %.6s**********\n", pan);
    printf("Valor: R$ %llu,%02llu\n", (unsigned long long)(amount / 100), (unsigned long long)(amount % 100));

    if (amount == 40800) {

        printf("\nSimulando timeout...\n");

        status = pos_app_process_timeout(app);

        if (status != POS_SUCCESS) {
            printf("Erro ao processar timeout: %d\n", status);
            return;
        }

        printf("Timeout detectado. Iniciando reversao...\n");

        status = pos_app_process_reversal_completed(app);

        if (status != POS_SUCCESS) {
            printf("Erro ao concluir reversao: %d\n", status);
            return;
        }

        printf("Reversao concluida com sucesso!\n");

        return;
    }

    /*
     * Simula a comunicacao com o autorizador.
     */

    const char *response_code = get_simulated_response_code(request.amount);

    status = simulate_authorization(&request_buffer, &response_buffer, response_code);

    if (status != POS_SUCCESS) {
        printf("Erro ao simular autorizacao: %d\n", status);
        return;
    }

    /*
     * Processa a resposta ISO8583 0210.
     */
    status = pos_app_process_response(app, response_buffer.data, response_buffer.length);

    if (status != POS_SUCCESS) {
        printf("Erro ao processar resposta: %d\n", status);
        return;
    }

    printf("\n================================\n");
    if (strcmp(response_code, "00") == 0) {
        printf("       VENDA APROVADA!\n");
    } else {
        printf("       VENDA NEGADA!\n");
    }
    printf("================================\n");

    /*
     * Libera a FSM para uma nova transacao.
     */
    status = pos_app_reset_transaction(app);

    if (status != POS_SUCCESS) {
        printf("Erro ao finalizar transacao: %d\n", status);
    }
}


static const char *get_transaction_status(LogTransactionStatus status) {

    switch (status) {

        case LOG_TRANSACTION_APPROVED:
            return "APROVADA";

        case LOG_TRANSACTION_DECLINED:
            return "NEGADA";

        case LOG_TRANSACTION_TIMEOUT:
            return "TIMEOUT";

        case LOG_TRANSACTION_REVERSED:
            return "REVERSAO";

        default:
            return "DESCONHECIDO";
    }
}


static void show_transaction_history(POSApp *app) {

    LogRecord record;
    POSStatus status;
    int records_found = 0;

    printf("\n======= HISTORICO DE TRANSACOES =======\n\n");

    status = log_reset_read(&app->log);

    if (status != POS_SUCCESS) {
        printf("Erro ao consultar historico: %d\n", status);
        return;
    }

    while (1) {

        status = log_read(&app->log, &record);

        if (status == POS_ERROR_LOG_NO_RECORD) {
            break;
        }

        if (status != POS_SUCCESS) {
            printf("Erro na leitura do registro: %d\n", status);
            break;
        }

        records_found++;

        printf("--------------------------------------\n");
        printf("Transacao: %lu\n",
               (unsigned long)record.transaction_id);

        printf("Data: %08lu\n",
               (unsigned long)record.timestamp);

        printf("Valor: R$ %lu,%02lu\n",
               (unsigned long)(record.amount / 100),
               (unsigned long)(record.amount % 100));

        printf("Status: %s\n",
               get_transaction_status(record.status));

        printf("PAN: %s\n", record.pan);

        printf("Codigo de resposta: %s\n",
               record.response_code);
    }

    if (records_found == 0) {
        printf("Nenhuma transacao encontrada.\n");
    }

    printf("--------------------------------------\n");
    printf("Total de registros: %d\n", records_found);
}


int main(void) {

    POSApp app;
    FlashStorage storage;

    char option[INPUT_SIZE];
    char *end_pointer;
    long selected_option;

    POSStatus status;

    /*
     * Inicializa a aplicacao uma unica vez.
     */
    status = pos_app_init(&app, &storage);

    if (status != POS_SUCCESS) {
        printf("Erro ao inicializar POS: %d\n", status);
        return EXIT_FAILURE;
    }

    /*
     * Configura o callback do relogio.
     * Deve ser configurado depois de pos_app_init().
     */
    status = pos_app_set_clock(&app, get_current_datetime, NULL);

    if (status != POS_SUCCESS) {
        printf("Erro ao configurar relogio: %d\n", status);
        return EXIT_FAILURE;
    }

    /*
     * Menu principal.
     */
    while (1) {

        show_menu();

        if (!read_line("Selecione uma opcao: ",
                       option, sizeof(option))) {
            break;
        }

        selected_option = strtol(option, &end_pointer, 10);

        if (option[0] == '\0' || *end_pointer != '\0') {
            printf("\nOpcao invalida.\n");
            continue;
        }

        switch (selected_option) {

            case 1:
                process_sale(&app);
                break;

            case 2:
                show_transaction_history(&app);
                break;

            case 3:
                printf("\nEncerrando POS...\n");
                return EXIT_SUCCESS;

            default:
                printf("\nOpcao invalida.\n");
                break;
        }
    }

    printf("\nEncerrando POS...\n");

    return EXIT_SUCCESS;
}