/**
 * @file pos_app.h
 * @author Gabriel Laurentino
 * @brief Implementação de integração e gerenciamento da aplicação POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 * 
 */

#include "pos_app.h"
#include "luhn.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static void clear_pan(POSApp *app) {
    memset(app->pan, 0, sizeof(app->pan));
}

static void clear_app(POSApp *app) {
    memset(app, 0, sizeof(*app));

}

POSStatus pos_app_set_clock(POSApp *app, POSClockCallback callback, void *context) {

    if (app == NULL || callback == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    app->clock_callback = callback;
    app->clock_context = context;

    return POS_SUCCESS;
}

POSStatus pos_app_init(POSApp *app, FlashStorage *storage) {

    if (app == NULL || storage == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    clear_app(app);

    transaction_init(&app->transaction);

    POSStatus status = flash_storage_init(storage);

    if (status != POS_SUCCESS) {
        return status;
    }

    app->storage = storage;

    status = log_init(&app->log, storage);

    if (status != POS_SUCCESS) {
        return status;
    }

    return POS_SUCCESS;
}


POSStatus pos_app_start_transaction(POSApp *app) {
    
    if (app == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    return transaction_process_event(&app->transaction, POS_EVENT_START);

}


POSStatus pos_app_card_read(POSApp *app, const char *pan) {

    if (app == NULL || pan == NULL) {
        return POS_ERROR_NULL_POINTER;
    } 

    if (app->transaction.state != POS_STATE_CARD) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    size_t pan_size = strlen(pan);

    if (pan_size == 0 || pan_size > ISO8583_SIZE_PAN) {
        return POS_ERROR_INVALID_LENGTH;
    }

    POSStatus status = validate_card_number(pan, pan_size + 1);

    if (status != POS_SUCCESS) {
        return status;
    }

    memcpy(app->pan, pan, pan_size + 1);

    status = transaction_process_event(&app->transaction, POS_EVENT_CARD_READ);

    if (status != POS_SUCCESS) {
        clear_pan(app);
        return status;
    }

    return POS_SUCCESS;
}

static int is_leap_year(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

static POSStatus validate_datetime(const POSDateTime *datetime) {

    if (datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (datetime->year < 2000 || datetime->year > 2099 ||
        datetime->month < 1 || datetime->month > 12 ||
        datetime->hour < 0 || datetime->hour > 23 ||
        datetime->minute < 0 || datetime->minute > 59 ||
        datetime->second < 0 || datetime->second > 59) {
            return POS_ERROR_APP_INVALID_DATETIME;
    }

    static const int days_por_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int max_days = days_por_month[datetime->month - 1];

    if (datetime->month == 2 && is_leap_year(datetime->year)) {
        max_days = 29;
    }

    if (datetime->day < 1 || datetime->day > max_days) {
        return POS_ERROR_APP_INVALID_DATETIME;
    }

    return POS_SUCCESS;
}

static POSStatus get_datetime(POSApp *app, POSDateTime *datetime) {

    if (app == NULL || datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->clock_callback == NULL) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    memset(datetime, 0, sizeof(*datetime));

    return app->clock_callback(datetime, app->clock_context);    
}


static POSStatus populate_time_fields(POSApp *app, ISO8583Message *message) {
    POSDateTime datetime;

    POSStatus status = get_datetime(app, &datetime);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = validate_datetime(&datetime);

    if (status != POS_SUCCESS) {
        return status;
    }

    int transmission_length = snprintf(
        message->transmission_date,
        sizeof(message->transmission_date),
        "%02d%02d%02d%02d%02d",
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second
    );

    if (transmission_length != ISO8583_SIZE_TRANSMISSION_DATE) {
        return POS_ERROR_APP_INVALID_TRANSMISSION_DATE;
    }

    int local_time_length = snprintf(
        message->local_time, 
        sizeof(message->local_time), 
        "%02d%02d%02d", 
        datetime.hour, 
        datetime.minute, 
        datetime.second
    );

    if (local_time_length != ISO8583_SIZE_LOCAL_TIME) {
        return POS_ERROR_APP_INVALID_LOCAL_TIME;
    }

    return POS_SUCCESS;
}

static POSStatus populate_stan(POSApp *app, ISO8583Message *message, uint32_t *next_stan) {

    if (app == NULL || message == NULL || next_stan == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    *next_stan = (app->stan_counter >= 999999) ? 1 : app->stan_counter + 1;

    int length = snprintf(message->stan, sizeof(message->stan), "%06u", (unsigned int)*next_stan);

    if (length != ISO8583_SIZE_STAN) {
        return POS_ERROR_APP_INVALID_STAN;
    }

    return POS_SUCCESS;
}


POSStatus pos_app_build_request(POSApp *app, const POSTransactionRequest *request, ISO8583Buffer *buffer) {
    
    if (app == NULL || request == NULL || buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->transaction.state != POS_STATE_ISO_SEND) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    if (app->pan[0] == '\0') {
        return POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED;
    }

    if (request->amount > 999999999999ULL) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    ISO8583Message message = {0};

    memcpy(message.mti, "0200", 5);

    memcpy(message.pan, app->pan, sizeof(message.pan));

    memcpy(message.processing_code, request->processing_code, sizeof(message.processing_code));

    snprintf(message.amount, sizeof(message.amount), "%012llu", (unsigned long long)request->amount);

    memcpy(message.terminal_id, request->terminal_id, sizeof(message.terminal_id));

    memcpy(message.currency_code, request->currency_code, sizeof(message.currency_code));

    POSStatus status = populate_time_fields(app, &message);

    if (status != POS_SUCCESS) {
        return status;
    }

    uint32_t next_stan;

    status = populate_stan(app, &message, &next_stan);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = iso8583_build(&message, buffer);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = transaction_process_event(&app->transaction, POS_EVENT_REQUEST_SENT);

    if (status != POS_SUCCESS) {
        return status;
    }

    app->stan_counter = next_stan;
    snprintf(app->amount, sizeof(app->amount), "%llu", (unsigned long long)request->amount);

    return POS_SUCCESS;
}

// static POSStatus parse_amount(const char *amount, uint32_t *value) {

//     if (amount == NULL || value == NULL) {
//         return POS_ERROR_NULL_POINTER;
//     }

//     if (strlen(amount) != ISO8583_SIZE_AMOUNT) {
//         return POS_ERROR_INVALID_FIELD_LENGTH;
//     }

//     uint32_t result = 0;

//     for (size_t i = 0; i < ISO8583_SIZE_AMOUNT; i++) {

//         if (amount[i] < '0' || amount[i] > '9') {
//             return POS_ERROR_INVALID_FIELD_FORMAT;
//         }

//         uint32_t digit = (uint32_t)(amount[i] - '0');

//         if (result > (UINT32_MAX - digit) / 10) {
//             return POS_ERROR_INVALID_ARGUMENT;
//         }

//         result = result * 10 + digit;
//     }

//     *value = result;

//     return POS_SUCCESS;
// }

static POSStatus get_log_timestamp(POSApp *app, uint32_t *timestamp) {

    if (app == NULL || timestamp == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    POSDateTime datetime = {0};

    POSStatus status = get_datetime(app, &datetime);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = validate_datetime(&datetime);

    if (status != POS_SUCCESS) {
        return status;
    }

    *timestamp = (uint32_t)(datetime.year * 10000 + datetime.month * 100 + datetime.day);

    return POS_SUCCESS;
}

POSStatus pos_app_process_response(POSApp *app, const unsigned char *data, size_t data_size) {

    if (app == NULL || data == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->transaction.state != POS_STATE_WAIT_RESP || app->transaction.reversal_required) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    ISO8583Message response = {0};

    POSStatus status = iso8583_parse(data, data_size, &response);

    if (status != POS_SUCCESS) {
        return status;
    }

    if (strcmp(response.mti, "0210") != 0) {
        return POS_ERROR_INVALID_MTI;
    }

    char expected_stan[ISO8583_SIZE_STAN + 1];

    int length = snprintf(expected_stan, sizeof(expected_stan), "%06u", (unsigned int)app->stan_counter);

    if (length != ISO8583_SIZE_STAN) {
        return POS_ERROR_APP_INVALID_STAN;
    }

    if (strcmp(response.stan, expected_stan) != 0) {
        return POS_ERROR_INVALID_FIELD;
    }

    if (strlen(response.response_code) != ISO8583_SIZE_RESPONSE_CODE) {
        return POS_ERROR_INVALID_FIELD_LENGTH;
    }

    int approved = strcmp(response.response_code, "00") == 0;

    LogRecord record = {0};

    status = get_log_timestamp(app, &record.timestamp);

    if (status != POS_SUCCESS) {
        return status;
    }

    record.transaction_id = app->stan_counter;

    record.amount = (uint64_t)strtoul(app->amount, NULL, 10);

    record.status = approved ? LOG_TRANSACTION_APPROVED : LOG_TRANSACTION_DECLINED;

    memcpy(record.pan, app->pan, sizeof(record.pan));
    memcpy(record.response_code, response.response_code, sizeof(record.response_code));

    for (size_t i = 0; i < ISO8583_SIZE_RESPONSE_CODE; i++) {
        if (response.response_code[i] < '0' || response.response_code[i] > '9') {
            return POS_ERROR_INVALID_FIELD_FORMAT;
        }
    }

    status = transaction_process_event(&app->transaction, approved ? POS_EVENT_RESPONSE_APPROVED : POS_EVENT_RESPONSE_DECLINED);

    if (status != POS_SUCCESS) {
        return status;
    }

    return log_write(&app->log, &record);

}

static POSStatus build_event_log_record(POSApp *app, LogTransactionStatus transaction_status, const char *response_code, LogRecord *record) {
    
    if (app == NULL || response_code == NULL || record == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->pan[0] == '\0') {
        return POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED;
    }

    if (app->amount[0] == '\0') {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    memset(record, 0, sizeof(*record));

    POSStatus status = get_log_timestamp(app, &record->timestamp);

    if (status != POS_SUCCESS) {
        return status;
    }

    record->amount = (uint64_t)strtoul(app->amount, NULL, 10);

    record->transaction_id = app->stan_counter;
    record->status = transaction_status;

    memcpy(record->pan, app->pan, sizeof(record->pan));

    memcpy(record->response_code, response_code, LOG_RESPONSE_CODE_SIZE);

    record->response_code[LOG_RESPONSE_CODE_SIZE] = '\0';

    return POS_SUCCESS;
}

POSStatus pos_app_process_timeout(POSApp *app) {

    if (app == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->transaction.state != POS_STATE_WAIT_RESP || app->transaction.reversal_required) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    LogRecord record;

    POSStatus status = build_event_log_record(app, LOG_TRANSACTION_TIMEOUT, "TO", &record);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = transaction_process_event(&app->transaction, POS_EVENT_TIMEOUT);

    if (status != POS_SUCCESS) {
        return status;
    }

    return log_write(&app->log, &record);    
}

POSStatus pos_app_process_reversal_completed(POSApp *app) {

    if (app == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->transaction.state != POS_STATE_REVERSAL || !app->transaction.reversal_required) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    LogRecord record;

    POSStatus status = build_event_log_record(app, LOG_TRANSACTION_REVERSED, "RV", &record);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = transaction_process_event(&app->transaction, POS_EVENT_REVERSAL_COMPLETED);

    if (status != POS_SUCCESS) {
        return status;
    }
    
    return log_write(&app->log, &record);
}

POSStatus pos_app_reset_transaction(POSApp *app) {

    if (app == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (app->transaction.state != POS_STATE_CARD && 
        app->transaction.state != POS_STATE_ISO_SEND &&
        app->transaction.state != POS_STATE_APPROVED && 
        app->transaction.state != POS_STATE_DECLINED) {
        return POS_ERROR_APP_INVALID_STATE;
    }

    POSStatus status = transaction_process_event(&app->transaction, POS_EVENT_RESET);

    if (status != POS_SUCCESS) {
        return status;
    }

    clear_pan(app);
    memset(app->amount, 0, sizeof(app->amount));

    return POS_SUCCESS;
}