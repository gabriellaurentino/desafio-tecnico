#include "test_utils.h"
#include "pos_app.h"

#include <string.h>

static POSStatus integration_clock(POSDateTime *datetime, void *context) {
    (void)context;

    if (datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    datetime->year = 2026;
    datetime->month = 11;
    datetime->day = 15;
    datetime->hour = 10;
    datetime->minute = 30;
    datetime->second = 45;

    return POS_SUCCESS;
}


void test_pos_app_transaction_build_request(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};
    ISO8583Message parsed = {0};

    POSTransactionRequest request = {0};

    strcpy(request.processing_code, "000000");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    POSStatus status = pos_app_init(&app, &storage);

    assert_equal("INTEGRATION - INIT", POS_SUCCESS, status);

    status = pos_app_set_clock(&app, integration_clock, NULL);

    assert_equal("INTEGRATION - SET CLOCK", POS_SUCCESS, status);

    status = pos_app_start_transaction(&app);

    assert_equal("INTEGRATION - START", POS_SUCCESS, status);

    assert_equal("INTEGRATION - CARD STATE", POS_STATE_CARD, app.transaction.state);

    status = pos_app_card_read(&app, "4532015112830366");

    assert_equal("INTEGRATION - CARD READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - ISO SEND STATE", POS_STATE_ISO_SEND, app.transaction.state);

    status = pos_app_build_request(&app, &request, &buffer);

    assert_equal("INTEGRATION - BUILD REQUEST", POS_SUCCESS, status);

    assert_equal("INTEGRATION - WAIT RESPONSE STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    assert_equal("INTEGRATION - STAN COUNTER", 1, (int)app.stan_counter);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("INTEGRATION - PARSE RESPONSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - MTI", 0, strcmp(parsed.mti, "0200"));

    assert_equal("INTEGRATION - PAN", 0, strcmp(parsed.pan, "4532015112830366"));

    assert_equal("INTEGRATION - PROCESSING CODE", 0, strcmp(parsed.processing_code, "000000"));

    assert_equal("INTEGRATION - AMOUNT", 0, strcmp(parsed.amount, "000000010000"));

    assert_equal("INTEGRATION - STAN", 0, strcmp(parsed.stan, "000001"));

    assert_equal("INTEGRATION - TRANSMISSION DATE", 0, strcmp(parsed.transmission_date, "1115103045"));

    assert_equal("INTEGRATION - LOCAL TIME", 0, strcmp(parsed.local_time, "103045"));

    assert_equal("INTEGRATION - TERMINAL ID", 0, strcmp(parsed.terminal_id, "TERM0001"));

    assert_equal("INTEGRATION - CURRENCY", 0, strcmp(parsed.currency_code, "986"));
}

static void prepare_response_test(POSApp *app, FlashStorage *storage, ISO8583Buffer *request_buffer) {
    POSTransactionRequest request = {0};

    strcpy(request.processing_code, "000000");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    POSStatus status = pos_app_init(app, storage);
    assert_equal("INTEGRATION - RESPONSE - INIT", POS_SUCCESS, status);

    status = pos_app_set_clock(app, integration_clock, NULL);
    assert_equal("INTEGRATION - RESPONSE - SET CLOCK", POS_SUCCESS, status);

    status = pos_app_start_transaction(app);
    assert_equal("INTEGRATION - RESPONSE - START", POS_SUCCESS, status);

    status = pos_app_card_read(app, "4532015112830366");
    assert_equal("INTEGRATION - RESPONSE - CARD READ", POS_SUCCESS, status);

    status = pos_app_build_request(app, &request, request_buffer);
    assert_equal("INTEGRATION - RESPONSE - BUILD REQUEST", POS_SUCCESS, status);
}


static POSStatus build_test_response(const ISO8583Buffer *request_buffer, ISO8583Buffer *response_buffer, const char *mti, const char *stan, const char *response_code) {
    ISO8583Message message = {0};

    POSStatus status = iso8583_parse(request_buffer->data, request_buffer->length, &message);

    if (status != POS_SUCCESS) {
        return status;
    }

    strcpy(message.mti, mti);
    strcpy(message.stan, stan);

    if (response_code != NULL) {
        strcpy(message.response_code, response_code);
    }

    return iso8583_build(&message, response_buffer);
}

void test_pos_app_transaction_response_approved(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    LogRecord record = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - APPROVED - BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - APPROVED - PROCESS RESPONSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - APPROVED - TRANSACTION STATE", POS_STATE_APPROVED, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APPROVED - LOG READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - APPROVED - TIMESTAMP", 20261115, (int)record.timestamp);
    assert_equal("INTEGRATION - APPROVED - TRANSACTION ID", 1, (int)record.transaction_id);
    assert_equal("INTEGRATION - APPROVED - AMOUNT", 10000, (int)record.amount);

    assert_equal("INTEGRATION - APPROVED - LOG STATUS", LOG_TRANSACTION_APPROVED, record.status);

    assert_equal("INTEGRATION - APPROVED - MASKED PAN", 0, strcmp(record.pan, "453201******0366"));

    assert_equal("INTEGRATION - APPROVED - RESPONSE CODE", 0, strcmp(record.response_code, "00"));
}

void test_pos_app_transaction_response_declined(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    LogRecord record = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "05");

    assert_equal("INTEGRATION - DECLINED - BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - DECLINED - PROCESS RESPONSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - DECLINED - TRANSACTION STATE", POS_STATE_DECLINED, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - DECLINED - LOG READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - DECLINED - LOG STATUS", LOG_TRANSACTION_DECLINED, record.status);

    assert_equal("INTEGRATION - DECLINED - RESPONSE CODE", 0, strcmp(record.response_code, "05"));
}

void test_pos_app_transaction_response_invalid_mti(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    LogRecord record = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0200", "000001", "00");

    assert_equal("INTEGRATION - INVALID MTI - BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - INVALID MTI - STATUS", POS_ERROR_INVALID_MTI, status);

    assert_equal("INTEGRATION - INVALID MTI - TRANSACTION STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - INVALID MTI - NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_invalid_stan(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    LogRecord record = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000002", "00");

    assert_equal("INTEGRATION - INVALID STAN - BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - INVALID STAN - STATUS", POS_ERROR_INVALID_FIELD, status);

    assert_equal("INTEGRATION - INVALID STAN - TRANSACTION STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - INVALID STAN - NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_missing_code(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    LogRecord record = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", NULL);

    assert_equal("INTEGRATION - MISSING CODE - BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - MISSING CODE - STATUS", POS_ERROR_INVALID_FIELD_LENGTH, status);

    assert_equal("INTEGRATION - MISSING CODE - TRANSACTION STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - MISSING CODE - NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_invalid_state(void) {

    POSApp app;
    FlashStorage storage;

    unsigned char data[] = {0x00};

    POSStatus status = pos_app_init(&app, &storage);

    assert_equal("INTEGRATION - INVALID STATE - INIT", POS_SUCCESS, status);

    status = pos_app_process_response(&app, data, sizeof(data));

    assert_equal("INTEGRATION - INVALID STATE - STATUS", POS_ERROR_APP_INVALID_STATE, status);
}

void test_pos_app_transaction_response_malformed(void) {
    POSApp app = {0};
    FlashStorage storage = {0};
    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - BUILD MALFORMED RESPONSE", POS_SUCCESS, status);

    response_buffer.data[12] = 'X';

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - MALFORMED RESPONSE STATUS", POS_ERROR_INVALID_FORMAT, status);
    
    assert_equal("INTEGRATION - MALFORMED RESPONSE STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    LogRecord record = {0};

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - MALFORMED RESPONSE NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_truncated(void) {
    POSApp app = {0};
    FlashStorage storage = {0};
    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - BUILD TRUNCATED RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length - 1);

    assert_equal("INTEGRATION - TRUNCATED RESPONSE STATUS", POS_ERROR_INVALID_FIELD_LENGTH, status);

    assert_equal("INTEGRATION - TRUNCATED RESPONSE STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    LogRecord record = {0};

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - TRUNCATED RESPONSE NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_invalid_code(void) {
    POSApp app = {0};
    FlashStorage storage = {0};
    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "A0");

    assert_equal("INTEGRATION - BUILD INVALID CODE RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - INVALID RESPONSE CODE STATUS", POS_ERROR_INVALID_FIELD_FORMAT, status);

    assert_equal("INTEGRATION - INVALID RESPONSE CODE STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    LogRecord record = {0};

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - INVALID RESPONSE CODE NO LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_response_log_failure(void) {
    POSApp app = {0};
    FlashStorage storage = {0};
    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - BUILD LOG FAILURE RESPONSE", POS_SUCCESS, status);

    app.log.write_address = FLASH_STORAGE_SIZE - LOG_STORAGE_RECORD_SIZE + 1;

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - LOG FAILURE STATUS", POS_ERROR_LOG_STORAGE_FULL, status);

    assert_equal("INTEGRATION - LOG FAILURE FINANCIAL STATE", POS_STATE_APPROVED, app.transaction.state);
}

void test_pos_app_transaction_response_duplicate(void) {
    POSApp app = {0};
    FlashStorage storage = {0};
    ISO8583Buffer request_buffer = {0};
    ISO8583Buffer response_buffer = {0};

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - BUILD DUPLICATE RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - FIRST RESPONSE STATUS", POS_SUCCESS, status);

    assert_equal("INTEGRATION - FIRST RESPONSE STATE", POS_STATE_APPROVED, app.transaction.state);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - DUPLICATE RESPONSE STATUS", POS_ERROR_APP_INVALID_STATE, status);

    LogRecord record = {0};

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - DUPLICATE RESPONSE FIRST LOG", POS_SUCCESS, status);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - DUPLICATE RESPONSE NO SECOND LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_process_timeout(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP TIMEOUT STATUS", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP TIMEOUT STATE", POS_STATE_REVERSAL, app.transaction.state);
    assert_equal("INTEGRATION - APP TIMEOUT REVERSAL REQUIRED", 1, app.transaction.reversal_required);

    LogRecord record;

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP TIMEOUT LOG READ", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP TIMEOUT LOG STATUS", LOG_TRANSACTION_TIMEOUT, record.status);
    assert_equal("INTEGRATION - APP TIMEOUT LOG TIMESTAMP", 20261115, record.timestamp);
    assert_equal("INTEGRATION - APP TIMEOUT LOG TRANSACTION ID", 1, record.transaction_id);
    assert_equal("INTEGRATION - APP TIMEOUT LOG AMOUNT", 10000, record.amount);
    assert_equal("INTEGRATION - APP TIMEOUT LOG RESPONSE CODE", 0, strcmp(record.response_code, "TO"));
    assert_equal("INTEGRATION - APP TIMEOUT LOG PAN", 0, strcmp(record.pan, "453201******0366"));
}

void test_pos_app_process_timeout_duplicate(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP TIMEOUT FIRST STATUS", POS_SUCCESS, status);

    status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP TIMEOUT DUPLICATE STATUS", POS_ERROR_APP_INVALID_STATE, status);

    LogRecord record;

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP TIMEOUT DUPLICATE LOG READ", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP TIMEOUT DUPLICATE LOG STATUS", LOG_TRANSACTION_TIMEOUT, record.status);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP TIMEOUT DUPLICATE NO EXTRA LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_process_timeout_invalid_state(void) {
    POSApp app;
    FlashStorage storage;

    POSStatus status = pos_app_init(&app, &storage);

    assert_equal("INTEGRATION - APP TIMEOUT INVALID INIT", POS_SUCCESS, status);

    status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP TIMEOUT INVALID STATE", POS_ERROR_APP_INVALID_STATE, status);
}

void test_pos_app_process_reversal_completed(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP REVERSAL TIMEOUT STATUS", POS_SUCCESS, status);

    status = pos_app_process_reversal_completed(&app);

    assert_equal("INTEGRATION - APP REVERSAL STATUS", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP REVERSAL STATE", POS_STATE_IDLE, app.transaction.state);
    assert_equal("INTEGRATION - APP REVERSAL REQUIRED", 0, app.transaction.reversal_required);

    LogRecord record;

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP REVERSAL TIMEOUT LOG READ", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP REVERSAL TIMEOUT LOG STATUS", LOG_TRANSACTION_TIMEOUT, record.status);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP REVERSAL LOG READ", POS_SUCCESS, status);
    assert_equal("INTEGRATION - APP REVERSAL LOG STATUS", LOG_TRANSACTION_REVERSED, record.status);
    assert_equal("INTEGRATION - APP REVERSAL LOG TIMESTAMP", 20261115, record.timestamp);
    assert_equal("INTEGRATION - APP REVERSAL LOG TRANSACTION ID", 1, record.transaction_id);
    assert_equal("INTEGRATION - APP REVERSAL LOG AMOUNT", 10000, record.amount);
    assert_equal("INTEGRATION - APP REVERSAL LOG RESPONSE CODE", 0, strcmp(record.response_code, "RV"));
    assert_equal("INTEGRATION - APP REVERSAL LOG PAN", 0, strcmp(record.pan, "453201******0366"));
}

void test_pos_app_process_reversal_without_timeout(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = pos_app_process_reversal_completed(&app);

    assert_equal("INTEGRATION - APP REVERSAL WITHOUT TIMEOUT", POS_ERROR_APP_INVALID_STATE, status);

    assert_equal("INTEGRATION - APP REVERSAL WITHOUT TIMEOUT STATE", POS_STATE_WAIT_RESP, app.transaction.state);

    assert_equal("INTEGRATION - APP REVERSAL WITHOUT TIMEOUT FLAG", 0, app.transaction.reversal_required);
}

void test_pos_app_process_timeout_log_failure(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    app.log.write_address = FLASH_STORAGE_SIZE - LOG_STORAGE_RECORD_SIZE + 1;

    POSStatus status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP TIMEOUT LOG FAILURE STATUS", POS_ERROR_LOG_STORAGE_FULL, status);

    assert_equal("INTEGRATION - APP TIMEOUT LOG FAILURE STATE", POS_STATE_REVERSAL, app.transaction.state);

    assert_equal("INTEGRATION - APP TIMEOUT LOG FAILURE REVERSAL", 1, app.transaction.reversal_required);

    assert_equal("INTEGRATION - APP TIMEOUT LOG FAILURE WRITE ADDRESS", FLASH_STORAGE_SIZE - LOG_STORAGE_RECORD_SIZE + 1, app.log.write_address);
}

void test_pos_app_process_reversal_log_failure(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE TIMEOUT", POS_SUCCESS, status);

    app.log.write_address = FLASH_STORAGE_SIZE - LOG_STORAGE_RECORD_SIZE + 1;

    status = pos_app_process_reversal_completed(&app);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE STATUS", POS_ERROR_LOG_STORAGE_FULL, status);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE STATE", POS_STATE_IDLE, app.transaction.state);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE FLAG", 0, app.transaction.reversal_required);

    LogRecord record;

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE TIMEOUT READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - APP REVERSAL LOG FAILURE TIMEOUT STATUS", LOG_TRANSACTION_TIMEOUT, record.status);

}

void test_pos_app_process_late_response_after_timeout(void) {
    POSApp app;
    FlashStorage storage;
    ISO8583Buffer request_buffer;
    ISO8583Buffer response_buffer;

    prepare_response_test(&app, &storage, &request_buffer);

    POSStatus status = build_test_response(&request_buffer, &response_buffer, "0210", "000001", "00");

    assert_equal("INTEGRATION - APP LATE RESPONSE BUILD", POS_SUCCESS, status);

    status = pos_app_process_timeout(&app);

    assert_equal("INTEGRATION - APP LATE RESPONSE TIMEOUT", POS_SUCCESS, status);

    status = pos_app_process_response(&app, response_buffer.data, response_buffer.length);

    assert_equal("INTEGRATION - APP LATE RESPONSE STATUS", POS_ERROR_APP_INVALID_STATE, status);

    assert_equal("INTEGRATION - APP LATE RESPONSE STATE", POS_STATE_REVERSAL, app.transaction.state);

    assert_equal("INTEGRATION - APP LATE RESPONSE REVERSAL", 1, app.transaction.reversal_required);

    LogRecord record;

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP LATE RESPONSE TIMEOUT LOG", POS_SUCCESS, status);

    assert_equal("INTEGRATION - APP LATE RESPONSE LOG STATUS", LOG_TRANSACTION_TIMEOUT, record.status);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - APP LATE RESPONSE NO EXTRA LOG", POS_ERROR_LOG_NO_RECORD, status);
}

void test_pos_app_transaction_sequential(void) {

    POSApp app;
    FlashStorage storage;

    ISO8583Buffer first_request = {0};
    ISO8583Buffer first_response = {0};

    ISO8583Buffer second_request = {0};
    ISO8583Buffer second_response = {0};

    ISO8583Message message = {0};

    LogRecord record = {0};

    POSTransactionRequest request = {0};

    strcpy(request.processing_code, "000000");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    prepare_response_test(&app, &storage, &first_request);

    POSStatus status = iso8583_parse(first_request.data, first_request.length,&message);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST REQUEST PARSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST STAN", 0, strcmp(message.stan, "000001"));

    status = build_test_response(&first_request, &first_response, "0210", "000001", "00");

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, first_response.data, first_response.length);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST PROCESS RESPONSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST APPROVED", POS_STATE_APPROVED, app.transaction.state);

    status = pos_app_reset_transaction(&app);

    assert_equal("INTEGRATION - SEQUENTIAL - RESET", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - RESET STATE", POS_STATE_IDLE, app.transaction.state);

    status = pos_app_start_transaction(&app);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND START", POS_SUCCESS, status);

    status = pos_app_card_read(&app, "4532015112830366");

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND CARD READ", POS_SUCCESS, status);

    status = pos_app_build_request(&app, &request, &second_request);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND BUILD REQUEST", POS_SUCCESS, status);

    memset(&message, 0, sizeof(message));

    status = iso8583_parse(second_request.data, second_request.length, &message);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND REQUEST PARSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND STAN", 0, strcmp(message.stan, "000002"));

    status = build_test_response(&second_request, &second_response, "0210", "000002", "00");

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND BUILD RESPONSE", POS_SUCCESS, status);

    status = pos_app_process_response(&app, second_response.data, second_response.length);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND PROCESS RESPONSE", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND APPROVED", POS_STATE_APPROVED, app.transaction.state);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST LOG READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST LOG ID", 1, (int)record.transaction_id);

    assert_equal("INTEGRATION - SEQUENTIAL - FIRST LOG STATUS", LOG_TRANSACTION_APPROVED, record.status);

    memset(&record, 0, sizeof(record));

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND LOG READ", POS_SUCCESS, status);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND LOG ID", 2, (int)record.transaction_id);

    assert_equal("INTEGRATION - SEQUENTIAL - SECOND LOG STATUS", LOG_TRANSACTION_APPROVED, record.status);

    status = log_read(&app.log, &record);

    assert_equal("INTEGRATION - SEQUENTIAL - NO MORE LOGS", POS_ERROR_LOG_NO_RECORD, status);

    assert_equal("INTEGRATION - SEQUENTIAL - STAN COUNTER", 2, (int)app.stan_counter);
}