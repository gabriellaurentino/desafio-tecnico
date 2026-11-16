#include <string.h>
#include "log.h"
#include "test_utils.h"


static LogRecord create_valid_log_record(void) {

    LogRecord record = {
        .timestamp = 1731528000,
        .transaction_id = 1001,
        .amount = 3,
        .status = LOG_TRANSACTION_APPROVED,
        .pan = "1234567890123456",
        .response_code = "00"
    };

    return record;
}

void test_log_init_success(void) {

    Log log;
    FlashStorage storage;

    flash_storage_init(&storage);

    POSStatus status = log_init(&log, &storage);

    assert_equal("LOG - INIT STATUS", POS_SUCCESS, status);
    assert_equal("LOG - INIT ADDRESS", 0, log.write_address);
    assert_equal("LOG - INIT STORAGE", 1, log.storage == &storage);
}

void test_log_init_null_log(void) {

    FlashStorage storage;

    flash_storage_init(&storage);

    POSStatus status = log_init(NULL, &storage);

    assert_equal("LOG - INIT NULL LOG", POS_ERROR_NULL_POINTER, status);
}

void test_log_init_null_storage(void) {

    Log log;

    POSStatus status = log_init(&log, NULL);

    assert_equal("LOG - INIT NULL STORAGE", POS_ERROR_NULL_POINTER, status);
}

void test_log_write_without_pan(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();
    
    strcpy(record.pan, "");

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE WITHOUT PAN STATUS", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("LOG - WRITE WITHOUT PAN ADDRESS", 0, log.write_address);
}

void test_log_write_with_pan(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE WITH PAN STATUS", POS_SUCCESS, status);

    assert_equal("LOG - WRITE WITH PAN ADDRESS", LOG_STORAGE_RECORD_SIZE, log.write_address);
}


void test_log_write_pan_not_exposed(void) {

    Log log;
    FlashStorage storage;

    const char *pan = "1234567890123456";

    LogRecord record = create_valid_log_record();

    flash_storage_init(&storage);
    log_init(&log, &storage);

    log_write(&log, &record);

    int found = 0;

    for (size_t i = 0; i <= FLASH_STORAGE_SIZE - 16; i++) {

        if (memcmp(&storage.data[i], pan, 16) == 0) {
            found = 1;
            break;
        }
    }

    assert_equal("LOG - PAN ORIGINAL NOT STORED", 0, found);
}

void test_log_write_xor_encryption(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - XOR WRITE STATUS", POS_SUCCESS, status);

    size_t stored_size = ((size_t)storage.data[0] << 8) | storage.data[1];

    assert_equal("LOG - XOR RECORD SIZE", LOG_RECORD_SIZE, stored_size);

    const unsigned char expected_record[LOG_RECORD_SIZE] = {
        0x67, 0x35, 0x05, 0x40,             /* Timestamp */
        0x00, 0x00, 0x03, 0xE9,             /* Transaction ID */
        0x00, 0x00, 0x00, 0x03,             /* Amount */
        LOG_TRANSACTION_APPROVED,           /* Status */
        '1', '2', '3', '4', '5', '6',       /* PAN */
        '*', '*', '*', '*', '*', '*',
        '3', '4', '5', '6',
        0x00, 0x00, 0x00,                  /* Padding */
        '0', '0'                            /* Response code */
    };

    int encrypted = 1;
    
    for (size_t i = 0; i < LOG_RECORD_SIZE; i++) {

        unsigned char expected_byte = expected_record[i] ^ LOG_XOR_KEY;

        if (storage.data[i + LOG_HEADER_SIZE] != expected_byte) {
            encrypted = 0;
            break;
        }
    }

    assert_equal("LOG - XOR RECORD ENCRYPTED", 1, encrypted);

    unsigned char decrypted[LOG_RECORD_SIZE];

    memcpy(decrypted, &storage.data[LOG_HEADER_SIZE], LOG_RECORD_SIZE);

    for (size_t i = 0; i < LOG_RECORD_SIZE; i++) {
        decrypted[i] ^= LOG_XOR_KEY;
    }

    assert_equal("LOG - XOR RECORD DECRYPTED", 0, memcmp(decrypted, expected_record, LOG_RECORD_SIZE));

    int masked_pan_found = 1;

    if (memcmp(&decrypted[LOG_TIMESTAMP_SIZE + LOG_TRANSACTION_ID_SIZE + LOG_AMOUNT_SIZE + LOG_STATUS_SIZE], "123456******3456", 16) != 0) {
        masked_pan_found = 0;
    }

    assert_equal("LOG - MASKED PAN AFTER DECRYPTION", 1, masked_pan_found);

    int original_pan_found = 0;

    for (size_t i = 0; i + 16 <= LOG_RECORD_SIZE; i++) {
        if (memcmp(&decrypted[i], "1234567890123456", 16) == 0) {
            original_pan_found = 1;
            break;
        }
    }

    assert_equal("LOG - ORIGINAL PAN AFTER DECRYPTION", 0, original_pan_found);
}

void test_log_write_null_log(void) {

    POSStatus status = log_write(NULL, NULL);

    assert_equal("LOG - WRITE NULL LOG", POS_ERROR_NULL_POINTER, status);
}

void test_log_write_null_storage(void) {

    Log log = {0};

    POSStatus status = log_write(&log, NULL);

    assert_equal("LOG - WRITE NULL STORAGE", POS_ERROR_NULL_POINTER, status);
}

void test_log_write_invalid_pan_length(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();
    
    strcpy(record.pan, "123456789012");

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE INVALID PAN TOO SHORT", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("LOG - WRITE INVALID PAN TOO SHORT ADDRESS", 0, log.write_address);

    memset(record.pan, '1', sizeof(record.pan));

    status = log_write(&log, &record);

    assert_equal("LOG - WRITE INVALID PAN TOO LONG", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("LOG - WRITE INVALID PAN TOO LONG ADDRESS", 0, log.write_address);
}

void test_log_write_invalid_pan_format(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();
    
    strcpy(record.pan, "123456789012ABCD");

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE INVALID PAN FORMAT", POS_ERROR_INVALID_FORMAT, status);
}

void test_log_write_storage_full(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();

    flash_storage_init(&storage);
    log_init(&log, &storage);

    log.write_address = FLASH_STORAGE_SIZE - 1;

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE STORAGE FULL", POS_ERROR_LOG_STORAGE_FULL, status);

    assert_equal("LOG - WRITE STORAGE FULL ADDRESS", FLASH_STORAGE_SIZE - 1, log.write_address);
}

void test_log_read_success(void) {

    Log log;
    FlashStorage storage;

    LogRecord record_write = create_valid_log_record();
    LogRecord record_read = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus write_status = log_write(&log, &record_write);

    assert_equal("LOG - READ SETUP WRITE", POS_SUCCESS, write_status);

    POSStatus status = log_read(&log, &record_read);

    assert_equal("LOG - READ SUCCESS STATUS", POS_SUCCESS, status);

    assert_equal("LOG - READ TIMESTAMP", record_write.timestamp, record_read.timestamp);

    assert_equal("LOG - READ TRANSACTION ID", record_write.transaction_id, record_read.transaction_id);

    assert_equal("LOG - READ AMOUNT", record_write.amount, record_read.amount);

    assert_equal("LOG - READ STATUS", record_write.status, record_read.status);

    assert_equal("LOG - READ PAN", 1, strcmp("123456******3456", record_read.pan) == 0);

    assert_equal("LOG - READ RESPONSE CODE", 1, strcmp(record_write.response_code, record_read.response_code) == 0);

    assert_equal("LOG - READ ADDRESS", log.write_address, log.read_address);
}

void test_log_read_multiple_records(void) {

    Log log;
    FlashStorage storage;

    LogRecord first_record = create_valid_log_record();
    LogRecord second_record = create_valid_log_record();

    LogRecord first_read = {0};
    LogRecord second_read = {0};

    second_record.timestamp = 1731528100;
    second_record.transaction_id = 1002;
    second_record.amount = 200;
    second_record.status = LOG_TRANSACTION_DECLINED;
    strcpy(second_record.response_code, "05");

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &first_record);

    assert_equal("LOG - MULTIPLE WRITE FIRST", POS_SUCCESS, status);

    status = log_write(&log, &second_record);

    assert_equal("LOG - MULTIPLE WRITE SECOND", POS_SUCCESS, status);

    status = log_read(&log, &first_read);

    assert_equal("LOG - MULTIPLE READ FIRST STATUS", POS_SUCCESS, status);

    assert_equal("LOG - MULTIPLE READ FIRST TIMESTAMP", first_record.timestamp, first_read.timestamp);

    assert_equal("LOG - MULTIPLE READ FIRST TRANSACTION ID", first_record.transaction_id, first_read.transaction_id);

    assert_equal("LOG - MULTIPLE READ FIRST STATUS VALUE", first_record.status, first_read.status);

    assert_equal("LOG - MULTIPLE READ FIRST PAN", 1, strcmp("123456******3456", first_read.pan) == 0);

    status = log_read(&log, &second_read);

    assert_equal("LOG - MULTIPLE READ SECOND STATUS", POS_SUCCESS, status);

    assert_equal("LOG - MULTIPLE READ SECOND TIMESTAMP", second_record.timestamp, second_read.timestamp);

    assert_equal("LOG - MULTIPLE READ SECOND TRANSACTION ID", second_record.transaction_id, second_read.transaction_id);

    assert_equal("LOG - MULTIPLE READ SECOND AMOUNT", second_record.amount, second_read.amount);

    assert_equal("LOG - MULTIPLE READ SECOND STATUS VALUE", second_record.status, second_read.status);

    assert_equal("LOG - MULTIPLE READ SECOND RESPONSE CODE", 1, strcmp(second_record.response_code, second_read.response_code) == 0);

    assert_equal("LOG - MULTIPLE READ FINAL ADDRESS", log.write_address, log.read_address);
}

void test_log_reset_read_success(void) {

    Log log;
    FlashStorage storage;

    LogRecord first_record = create_valid_log_record();
    LogRecord second_record = create_valid_log_record();
    LogRecord record_read = {0};

    second_record.transaction_id = 1002;
    second_record.amount = 200;

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &first_record);

    assert_equal("LOG - RESET WRITE FIRST", POS_SUCCESS, status);

    status = log_write(&log, &second_record);

    assert_equal("LOG - RESET WRITE SECOND", POS_SUCCESS, status);

    status = log_read(&log, &record_read);

    assert_equal("LOG - RESET READ FIRST", POS_SUCCESS, status);

    assert_equal("LOG - RESET FIRST RECORD ID", first_record.transaction_id, record_read.transaction_id);

    status = log_read(&log, &record_read);

    assert_equal("LOG - RESET READ SECOND", POS_SUCCESS, status);

    assert_equal("LOG - RESET SECOND RECORD ID", second_record.transaction_id, record_read.transaction_id);

    status = log_read(&log, &record_read);

    assert_equal("LOG - RESET READ END", POS_ERROR_LOG_NO_RECORD, status);

    status = log_reset_read(&log);

    assert_equal("LOG - RESET STATUS", POS_SUCCESS, status);

    assert_equal("LOG - RESET ADDRESS", 0, log.read_address);

    status = log_read(&log, &record_read);

    assert_equal("LOG - RESET REREAD FIRST", POS_SUCCESS, status);

    assert_equal("LOG - RESET REREAD FIRST ID", first_record.transaction_id, record_read.transaction_id);
}

void test_log_reset_read_null_log(void) {

    POSStatus status = log_reset_read(NULL);

    assert_equal("LOG - RESET NULL LOG", POS_ERROR_NULL_POINTER, status);
}

void test_log_reset_read_null_storage(void) {

    Log log = {0};

    POSStatus status = log_reset_read(&log);

    assert_equal("LOG - RESET NULL STORAGE", POS_ERROR_NULL_POINTER, status);
}

void test_log_write_invalid_response_code(void) {

    Log log;
    FlashStorage storage;

    LogRecord record = create_valid_log_record();

    strcpy(record.response_code, "05");

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE INVALID RESPONSE CODE", POS_ERROR_INVALID_FORMAT, status);

    assert_equal("LOG - WRITE INVALID RESPONSE ADDRESS", 0, log.write_address);
}

void test_log_read_invalid_record_size(void) {

    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    storage.data[0] = 0x00;
    storage.data[1] = 0x01;

    log.write_address = LOG_STORAGE_RECORD_SIZE;

    POSStatus status = log_read(&log, &record);

    assert_equal("LOG - READ INVALID RECORD SIZE", POS_ERROR_INVALID_LENGTH, status);

    assert_equal("LOG - READ INVALID RECORD SIZE ADDRESS", 0, log.read_address);
}

void test_log_write_invalid_status(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = create_valid_log_record();

    record.status = (LogTransactionStatus)99;

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE INVALID STATUS", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("LOG - WRITE INVALID STATUS ADDRESS", 0, log.write_address);
}

void test_log_write_response_code_invalid_length(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = create_valid_log_record();

    memcpy(record.response_code, "000", sizeof(record.response_code));

    flash_storage_init(&storage);
    log_init(&log, &storage);

    POSStatus status = log_write(&log, &record);

    assert_equal("LOG - WRITE RESPONSE CODE INVALID LENGTH", POS_ERROR_INVALID_FORMAT, status);
}

void test_log_read_incomplete_record(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    storage.data[0] = (unsigned char)((LOG_RECORD_SIZE >> 8) & 0xFF);
    storage.data[1] = (unsigned char)(LOG_RECORD_SIZE & 0xFF);

    log.write_address = LOG_HEADER_SIZE + LOG_RECORD_SIZE - 1;

    POSStatus status = log_read(&log, &record);

    assert_equal("LOG - READ INCOMPLETE RECORD", POS_ERROR_INVALID_LENGTH, status);

    assert_equal("LOG - READ INCOMPLETE RECORD ADDRESS", 0, log.read_address);
}

void test_log_read_invalid_status(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    LogRecord valid_record = create_valid_log_record();

    POSStatus status = log_write(&log, &valid_record);

    assert_equal("LOG - READ INVALID STATUS SETUP", POS_SUCCESS, status);

    /*
     * O status fica depois de:
     * timestamp + transaction_id + amount.
     */
    size_t status_offset = LOG_HEADER_SIZE + LOG_TIMESTAMP_SIZE + LOG_TRANSACTION_ID_SIZE + LOG_AMOUNT_SIZE;

    storage.data[status_offset] = (unsigned char)(99 ^ LOG_XOR_KEY);

    status = log_read(&log, &record);

    assert_equal("LOG - READ INVALID STATUS", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("LOG - READ INVALID STATUS ADDRESS", 0, log.read_address);
}

void test_log_read_invalid_masked_pan(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    LogRecord valid_record = create_valid_log_record();

    POSStatus status = log_write(&log, &valid_record);

    assert_equal("LOG - READ INVALID PAN SETUP", POS_SUCCESS, status);

    size_t pan_offset = LOG_HEADER_SIZE + LOG_TIMESTAMP_SIZE + LOG_TRANSACTION_ID_SIZE + LOG_AMOUNT_SIZE + LOG_STATUS_SIZE;

    /*
     * Altera um caractere que deveria ser numérico
     * entre os seis primeiros dígitos do PAN.
     */
    storage.data[pan_offset] ^= LOG_XOR_KEY;
    storage.data[pan_offset] = 'X' ^ LOG_XOR_KEY;

    status = log_read(&log, &record);

    assert_equal("LOG - READ INVALID MASKED PAN", POS_ERROR_INVALID_FORMAT, status);

    assert_equal("LOG - READ INVALID PAN ADDRESS", 0, log.read_address);
}

void test_log_read_invalid_response_code(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    LogRecord valid_record = create_valid_log_record();

    POSStatus status = log_write(&log, &valid_record);

    assert_equal("LOG - READ INVALID RESPONSE SETUP",
        POS_SUCCESS, status);

    size_t response_offset = LOG_HEADER_SIZE + LOG_TIMESTAMP_SIZE + LOG_TRANSACTION_ID_SIZE + LOG_AMOUNT_SIZE + LOG_STATUS_SIZE + LOG_PAN_SIZE;

    /*
     * Mantém status APPROVED, mas altera o código "00"
     * para "05".
     */
    storage.data[response_offset] = '0' ^ LOG_XOR_KEY;
    storage.data[response_offset + 1] = '5' ^ LOG_XOR_KEY;

    status = log_read(&log, &record);

    assert_equal("LOG - READ INVALID RESPONSE CODE", POS_ERROR_INVALID_FORMAT, status);

    assert_equal("LOG - READ INVALID RESPONSE ADDRESS", 0, log.read_address);
}

void test_log_read_invalid_address(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    LogRecord valid_record = create_valid_log_record();

    log_write(&log, &valid_record);

    log.read_address = log.write_address + 1;

    POSStatus status = log_read(&log, &record);

    assert_equal("LOG - READ INVALID ADDRESS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_log_read_incomplete_header(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    log.write_address = 1;

    POSStatus status = log_read(&log, &record);

    assert_equal("LOG READ - Incomplete header", POS_ERROR_INVALID_LENGTH, status);
}

void test_log_read_write_address_out_of_bounds(void) {
    Log log;
    FlashStorage storage;
    LogRecord record = {0};

    flash_storage_init(&storage);
    log_init(&log, &storage);

    log.write_address = FLASH_STORAGE_SIZE + 1;

    POSStatus status = log_read(&log, &record);

    assert_equal("LOG READ - Write address out of bounds", POS_ERROR_INVALID_ARGUMENT, status);
}
