#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pos_app.h"
#include "test_utils.h"


void test_pos_app_init_success(void) {

    POSApp app;
    FlashStorage storage;

    POSStatus status = pos_app_init(&app, &storage);

    assert_equal("POS APP - INIT STATUS", POS_SUCCESS, status);

    assert_equal("POS APP - INIT STATE", POS_STATE_IDLE, app.transaction.state);

    assert_equal("POS APP - INIT REVERSAL", 0, app.transaction.reversal_required);

    assert_equal("POS APP - INIT STORAGE", 1, app.storage == &storage);

    assert_equal("POS APP - INIT LOG STORAGE", 1, app.log.storage == &storage);

    assert_equal("POS APP - INIT LOG WRITE ADDRESS", 0, app.log.write_address);

    assert_equal("POS APP - INIT LOG READ ADDRESS", 0, app.log.read_address);

    assert_equal("POS APP - INIT FLASH MEMORY", 0xFF, storage.data[0]);

    assert_equal("POS APP - INIT FLASH LAST BYTE", 0xFF, storage.data[FLASH_STORAGE_SIZE - 1]);
}

void test_pos_app_init_null_app(void) {

    FlashStorage storage;

    POSStatus status = pos_app_init(NULL, &storage);

    assert_equal("POS APP - INIT NULL APP", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_init_null_storage(void) {

    POSApp app;

    POSStatus status = pos_app_init(&app, NULL);

    assert_equal("POS APP - INIT NULL STORAGE", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_start_transaction_success(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_start_transaction(&app);

    assert_equal("POS APP - START TRANSACTION STATUS", POS_SUCCESS, status);

    assert_equal("POS APP - START TRANSACTION STATE", POS_STATE_CARD, app.transaction.state);
}

void test_pos_app_start_transaction_null(void) {

    POSStatus status = pos_app_start_transaction(NULL);

    assert_equal("POS APP - START TRANSACTION NULL", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_start_transaction_already_started(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    pos_app_start_transaction(&app);

    POSStatus status = pos_app_start_transaction(&app);

    assert_equal("POS APP - START TRANSACTION ALREADY STARTED", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("POS APP - START TRANSACTION STATE UNCHANGED", POS_STATE_CARD, app.transaction.state);
}

void test_pos_app_card_read_success(void) {

    POSApp app;
    FlashStorage storage;

    const char *pan = "4532015112830366";

    pos_app_init(&app, &storage);
    pos_app_start_transaction(&app);

    POSStatus status = pos_app_card_read(&app, pan);

    assert_equal("POS APP - CARD READ STATUS", POS_SUCCESS, status);

    assert_equal("POS APP - CARD READ STATE", POS_STATE_ISO_SEND, app.transaction.state);

    assert_equal("POS APP - CARD READ PAN", 0, strcmp(pan, app.pan));
}

void test_pos_app_card_read_invalid_pan(void) {

    POSApp app;
    FlashStorage storage;

    const char *pan = "4532015112830367";

    pos_app_init(&app, &storage);
    pos_app_start_transaction(&app);

    POSStatus status = pos_app_card_read(&app, pan);

    assert_equal("POS APP - CARD READ INVALID PAN", POS_ERROR_INVALID_PAN, status);

    assert_equal("POS APP - CARD READ INVALID PAN STATE", POS_STATE_CARD, app.transaction.state);

    assert_equal("POS APP - CARD READ INVALID PAN EMPTY", 0, app.pan[0]);
}

void test_pos_app_card_read_null_app(void) {

    POSStatus status = pos_app_card_read(NULL, "4532015112830366");

    assert_equal("POS APP - CARD READ NULL APP", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_card_read_null_pan(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_card_read(&app, NULL);

    assert_equal("POS APP - CARD READ NULL PAN", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_card_read_empty_pan(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);
    pos_app_start_transaction(&app);

    POSStatus status = pos_app_card_read(&app, "");

    assert_equal("POS APP - CARD READ EMPTY PAN", POS_ERROR_INVALID_LENGTH, status);
}

void test_pos_app_card_read_pan_too_long(void) {

    POSApp app;
    FlashStorage storage;

    const char *pan = "12345678901234567890";

    pos_app_init(&app, &storage);
    pos_app_start_transaction(&app);

    POSStatus status = pos_app_card_read(&app, pan);

    assert_equal("POS APP - CARD READ PAN TOO LONG", POS_ERROR_INVALID_LENGTH, status);
}

void test_pos_app_card_read_invalid_state(void) {

    POSApp app;
    FlashStorage storage;

    const char *pan = "4532015112830366";

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_card_read(&app, pan);

    assert_equal("POS APP - CARD READ INVALID STATE", POS_ERROR_APP_INVALID_STATE, status);

    assert_equal("POS APP - CARD READ INVALID STATE UNCHANGED", POS_STATE_IDLE, app.transaction.state);

    assert_equal("POS APP - CARD READ INVALID STATE EMPTY PAN", 0, app.pan[0]);
}

static POSStatus test_clock_success(POSDateTime *datetime, void *context) {
    (void)context;

    if (datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    datetime->year = 2026;
    datetime->month = 11;
    datetime->day = 14;
    datetime->hour = 15;
    datetime->minute = 30;
    datetime->second = 45;

    return POS_SUCCESS;
}

static POSStatus test_clock_invalid_datetime(POSDateTime *datetime, void *context) {
    (void)context;

    if (datetime == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    datetime->year = 2026;
    datetime->month = 13;
    datetime->day = 14;
    datetime->hour = 15;
    datetime->minute = 30;
    datetime->second = 45;

    return POS_SUCCESS;
}

static POSStatus test_clock_failure(POSDateTime *datetime, void *context) {
    (void)datetime;
    (void)context;

    return POS_ERROR_INVALID_ARGUMENT;
}

void test_pos_app_build_request_clock_failure(void) {

    POSApp app;
    FlashStorage storage;
    POSTransactionRequest request = {0};
    ISO8583Buffer buffer = {0};

    POSStatus status = pos_app_init(&app, &storage);
    assert_equal("POS APP - CLOCK FAILURE INIT", POS_SUCCESS, status);

    status = pos_app_set_clock(&app, test_clock_failure, NULL);
    assert_equal("POS APP - CLOCK FAILURE SET CALLBACK", POS_SUCCESS, status);

    status = pos_app_start_transaction(&app);
    assert_equal("POS APP - CLOCK FAILURE START", POS_SUCCESS, status);

    status = pos_app_card_read(&app, "4532015112830366");
    assert_equal("POS APP - CLOCK FAILURE CARD READ", POS_SUCCESS, status);

    strcpy(request.processing_code, "000000");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    status = pos_app_build_request(&app, &request, &buffer);

    assert_equal("POS APP - BUILD REQUEST CLOCK FAILURE", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("POS APP - CLOCK FAILURE STAN COUNTER", 0, (int)app.stan_counter);

    assert_equal("POS APP - CLOCK FAILURE STATE", POS_STATE_ISO_SEND, app.transaction.state);
}

void test_pos_app_set_clock_success(void) {
    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_set_clock(&app, test_clock_success, NULL);

    assert_equal("POS APP - SET CLOCK STATUS", POS_SUCCESS, status);
}

void test_pos_app_set_clock_null_app(void) {
    POSStatus status = pos_app_set_clock(NULL,test_clock_success, NULL);

    assert_equal("POS APP - SET CLOCK NULL APP", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_set_clock_null_callback(void) {
    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_set_clock(&app, NULL, NULL);

    assert_equal("POS APP - SET CLOCK NULL CALLBACK", POS_ERROR_NULL_POINTER, status);
}

static void prepare_app_for_stan_test(POSApp *app, FlashStorage *storage) {
    POSStatus status = pos_app_init(app, storage);
    assert_equal("STAN - PREPARE INIT", POS_SUCCESS, status);
    
    status = pos_app_set_clock(app, test_clock_success, NULL);
    assert_equal("STAN - PREPARE CLOCK", POS_SUCCESS, status);

    status = pos_app_start_transaction(app);
    assert_equal("STAN - PREPARE START", POS_SUCCESS, status);

    status = pos_app_card_read(app, "4532015112830366");
    assert_equal("STAN - PREPARE CARD", POS_SUCCESS, status);
}

static POSStatus build_stan_test_request(POSApp *app, ISO8583Buffer *buffer) {
    POSTransactionRequest request = {0};

    strcpy(request.processing_code, "000000");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    return pos_app_build_request(app, &request, buffer);
}

void test_pos_app_build_request_stan_first(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};
    ISO8583Message parsed = {0};

    prepare_app_for_stan_test(&app, &storage);

    POSStatus status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - FIRST BUILD", POS_SUCCESS, status);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("POS APP - FIRST PARSE", POS_SUCCESS, status);
    assert_equal("POS APP - FIRST VALUE", 1, atoi(parsed.stan));
    assert_equal("POS APP - FIRST LENGTH", 6, (int)strlen(parsed.stan));
    assert_equal("POS APP - FIRST COUNTER", 1, (int)app.stan_counter);
}

void test_pos_app_build_request_stan_next(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};
    ISO8583Message parsed = {0};

    prepare_app_for_stan_test(&app, &storage);

    app.stan_counter = 1;

    POSStatus status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - NEXT BUILD", POS_SUCCESS, status);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("POS APP - NEXT PARSE", POS_SUCCESS, status);
    assert_equal("POS APP - NEXT VALUE", 2, atoi(parsed.stan));
    assert_equal("POS APP - NEXT LENGTH", 6, (int)strlen(parsed.stan));
    assert_equal("POS APP - NEXT COUNTER", 2, (int)app.stan_counter);
}

void test_pos_app_build_request_stan_rollover(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};
    ISO8583Message parsed = {0};

    prepare_app_for_stan_test(&app, &storage);

    app.stan_counter = 999999;

    POSStatus status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - ROLLOVER BUILD", POS_SUCCESS, status);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("POS APP - ROLLOVER PARSE", POS_SUCCESS, status);
    assert_equal("POS APP - ROLLOVER VALUE", 1, atoi(parsed.stan));
    assert_equal("POS APP - ROLLOVER LENGTH", 6, (int)strlen(parsed.stan));
    assert_equal("POS APP - ROLLOVER COUNTER", 1, (int)app.stan_counter);
}

void test_pos_app_build_request_stan_last_value(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};
    ISO8583Message parsed = {0};

    prepare_app_for_stan_test(&app, &storage);

    app.stan_counter = 999998;

    POSStatus status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - LAST VALUE BUILD", POS_SUCCESS, status);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("POS APP - LAST VALUE PARSE", POS_SUCCESS, status);
    assert_equal("POS APP - LAST VALUE", 999999, atoi(parsed.stan));
    assert_equal("POS APP - LAST COUNTER", 999999, (int)app.stan_counter);
    assert_equal("POS APP - SIX DIGITS", 6, (int)strlen(parsed.stan));
}

void test_pos_app_build_request_stan_without_clock(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};

    POSStatus status = pos_app_init(&app, &storage);
    assert_equal("POS APP - NO CLOCK INIT", POS_SUCCESS, status);

    status = pos_app_start_transaction(&app);
    assert_equal("POS APP - NO CLOCK START", POS_SUCCESS, status);

    status = pos_app_card_read(&app, "4532015112830366");
    assert_equal("POS APP - NO CLOCK CARD READ", POS_SUCCESS, status);

    app.stan_counter = 10;

    status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - NO CLOCK ERROR", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("POS APP - NO CLOCK COUNTER", 10, (int)app.stan_counter);
    assert_equal("POS APP - NO CLOCK STATE", POS_STATE_ISO_SEND, app.transaction.state);
}


void test_pos_app_build_request_stan_invalid_datetime(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};

    prepare_app_for_stan_test(&app, &storage);

    POSStatus status = pos_app_set_clock(&app, test_clock_invalid_datetime, NULL);

    assert_equal("POS APP - INVALID DATE SET CLOCK", POS_SUCCESS, status);

    app.stan_counter = 20;

    status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - INVALID DATE ERROR", POS_ERROR_APP_INVALID_DATETIME, status);

    assert_equal("POS APP - INVALID DATE COUNTER", 20, (int)app.stan_counter);
    assert_equal("POS APP - INVALID DATE STATE", POS_STATE_ISO_SEND, app.transaction.state);
}

void test_pos_app_build_request_stan_without_pan(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};

    POSStatus status = pos_app_init(&app, &storage);
    assert_equal("POS APP - NO PAN INIT", POS_SUCCESS, status);

    status = pos_app_start_transaction(&app);
    assert_equal("POS APP - NO PAN START", POS_SUCCESS, status);

    status = transaction_process_event(&app.transaction, POS_EVENT_CARD_READ);

    assert_equal("POS APP - NO PAN FSM", POS_SUCCESS, status);

    app.stan_counter = 30;

    status = build_stan_test_request(&app, &buffer);

    assert_equal("POS APP - NO PAN ERROR", POS_ERROR_APP_CARD_NUMBER_NOT_INFORMED, status);

    assert_equal("POS APP - NO PAN COUNTER", 30, (int)app.stan_counter);
    assert_equal("POS APP - NO PAN STATE", POS_STATE_ISO_SEND, app.transaction.state);
}

void test_pos_app_build_request_stan_iso_build_failure(void) {

    POSApp app;
    FlashStorage storage;
    ISO8583Buffer buffer = {0};

    prepare_app_for_stan_test(&app, &storage);

    POSTransactionRequest request = {0};

    strcpy(request.processing_code, "0");
    request.amount = 10000;
    strcpy(request.terminal_id, "TERM0001");
    strcpy(request.currency_code, "986");

    app.stan_counter = 40;

    memset(&buffer, 0xAA, sizeof(buffer));

    ISO8583Buffer original_buffer = buffer;

    POSStatus status = pos_app_build_request(&app, &request, &buffer);

    assert_equal("POS APP - STAN - ISO BUILD FAILURE", POS_ERROR_INVALID_FIELD_LENGTH, status);

    assert_equal("POS APP - STAN - ISO BUILD COUNTER", 40, (int)app.stan_counter);

    assert_equal("POS APP - STAN - ISO BUILD STATE", POS_STATE_ISO_SEND, app.transaction.state);

    assert_equal("POS APP - STAN - BUFFER INTEGRITY", 0, memcmp(&buffer, &original_buffer, sizeof(buffer)));
}

void test_pos_app_reset_transaction_null_pointer(void) {

    POSStatus status = pos_app_reset_transaction(NULL);

    assert_equal("POS APP - RESET NULL POINTER", POS_ERROR_NULL_POINTER, status);
}

void test_pos_app_reset_transaction_invalid_state(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET INVALID STATE", POS_ERROR_APP_INVALID_STATE, status);

    assert_equal("POS APP - RESET KEEPS IDLE", POS_STATE_IDLE, app.transaction.state);
}

void test_pos_app_reset_transaction_approved(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    app.transaction.state = POS_STATE_APPROVED;

    strcpy(app.pan, "4532015112830366");
    strcpy(app.amount, "000000010000");

    app.stan_counter = 123;

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET APPROVED STATUS", POS_SUCCESS, status);

    assert_equal("POS APP - RESET APPROVED STATE", POS_STATE_IDLE, app.transaction.state);

    assert_equal("POS APP - RESET APPROVED PAN CLEARED", '\0', app.pan[0]);

    assert_equal("POS APP - RESET APPROVED AMOUNT CLEARED", '\0', app.amount[0]);

    assert_equal("POS APP - RESET APPROVED STAN PRESERVED", 123, app.stan_counter);
}

void test_pos_app_reset_transaction_declined(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    app.transaction.state = POS_STATE_DECLINED;

    strcpy(app.pan, "4532015112830366");
    strcpy(app.amount, "000000010000");

    app.stan_counter = 456;

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET DECLINED STATUS", POS_SUCCESS, status);

    assert_equal("POS APP - RESET DECLINED STATE", POS_STATE_IDLE, app.transaction.state);

    assert_equal("POS APP - RESET DECLINED PAN CLEARED", '\0', app.pan[0]);

    assert_equal("POS APP - RESET DECLINED AMOUNT CLEARED", '\0', app.amount[0]);

    assert_equal("POS APP - RESET DECLINED STAN PRESERVED", 456, app.stan_counter);
}

void test_pos_app_reset_transaction_waiting_response(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    app.transaction.state = POS_STATE_WAIT_RESP;

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET WAIT RESP STATUS", POS_ERROR_APP_INVALID_STATE, status);

    assert_equal("POS APP - RESET WAIT RESP STATE", POS_STATE_WAIT_RESP, app.transaction.state);
}

void test_pos_app_reset_transaction_card(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    pos_app_start_transaction(&app);

    strcpy(app.amount, "000000010000");

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET CARD STATUS", POS_SUCCESS, status);
    assert_equal("POS APP - RESET CARD STATE", POS_STATE_IDLE, app.transaction.state);
    assert_equal("POS APP - RESET CARD PAN CLEARED", '\0', app.pan[0]);
    assert_equal("POS APP - RESET CARD AMOUNT CLEARED", '\0', app.amount[0]);
}

void test_pos_app_reset_transaction_iso_send(void) {

    POSApp app;
    FlashStorage storage;

    pos_app_init(&app, &storage);

    pos_app_start_transaction(&app);
    pos_app_card_read(&app, "4532015112830366");

    strcpy(app.amount, "000000010000");

    POSStatus status = pos_app_reset_transaction(&app);

    assert_equal("POS APP - RESET ISO SEND STATUS", POS_SUCCESS, status);
    assert_equal("POS APP - RESET ISO SEND STATE", POS_STATE_IDLE, app.transaction.state);
    assert_equal("POS APP - RESET ISO SEND PAN CLEARED", '\0', app.pan[0]);
    assert_equal("POS APP - RESET ISO SEND AMOUNT CLEARED", '\0', app.amount[0]);
}
