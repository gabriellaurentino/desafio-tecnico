#include "flash_storage.h"
#include "test_utils.h"


void test_flash_storage_init(void) {

    FlashStorage storage;

    POSStatus status = flash_storage_init(&storage);

    assert_equal("FLASH STORAGE - INIT STATUS", POS_SUCCESS, status);
}

void test_flash_storage_init_memory(void) {

    FlashStorage storage;

    flash_storage_init(&storage);

    int valid = 1;

    for (size_t i = 0; i < FLASH_STORAGE_SIZE; i++) {
        if (storage.data[i] != 0xFF) {
            valid = 0;
            break;
        }
    }
    assert_equal("FLASH STORAGE - INIT MEMORY", 1, valid);
}

void test_flash_storage_init_null(void) {

    POSStatus status = flash_storage_init(NULL);

    assert_equal("FLASH STORAGE - INIT NULL", POS_ERROR_NULL_POINTER, status);
}

void test_flash_storage_read_empty(void) {

    FlashStorage storage;
    unsigned char buffer[5];

    flash_storage_init(&storage);

    POSStatus status = flash_storage_read(&storage, 0, buffer, sizeof(buffer));

    int valid = 1;

    for (size_t i = 0; i < sizeof(buffer); i++) {
        if (buffer[i] != 0xFF) {
            valid = 0;
            break;
        }
    }

    assert_equal("FLASH STORAGE - READ EMPTY STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - READ EMPTY DATA", 1, valid);
}

void test_flash_storage_read_address(void) {

    FlashStorage storage;
    unsigned char buffer[3];

    flash_storage_init(&storage);

    storage.data[10] = 0xAA;
    storage.data[11] = 0xBB;
    storage.data[12] = 0xCC;

    POSStatus status = flash_storage_read(&storage, 10, buffer, sizeof(buffer));

    assert_equal("FLASH STORAGE - READ ADDRESS STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - READ ADDRESS BYTE 1", 0xAA, buffer[0]);
    assert_equal("FLASH STORAGE - READ ADDRESS BYTE 2", 0xBB, buffer[1]);
    assert_equal("FLASH STORAGE - READ ADDRESS BYTE 3", 0xCC, buffer[2]);
}

void test_flash_storage_read_null(void) {

    FlashStorage storage;
    unsigned char buffer[5];

    flash_storage_init(&storage);

    POSStatus status_storage = flash_storage_read(NULL, 0, buffer, sizeof(buffer));
    POSStatus status_buffer = flash_storage_read(&storage, 0, NULL, sizeof(buffer));

    assert_equal("FLASH STORAGE - READ NULL STORAGE", POS_ERROR_NULL_POINTER, status_storage);
    assert_equal("FLASH STORAGE - READ NULL BUFFER", POS_ERROR_NULL_POINTER, status_buffer);
}

void test_flash_storage_read_invalid_size(void) {

    FlashStorage storage;
    unsigned char buffer[FLASH_STORAGE_MAX_OPERATION_SIZE + 1];

    flash_storage_init(&storage);

    POSStatus status_zero = flash_storage_read(&storage, 0, buffer, 0);

    POSStatus status_exceeded = flash_storage_read(&storage, 0, buffer, sizeof(buffer));

    assert_equal("FLASH STORAGE - READ ZERO SIZE", POS_ERROR_INVALID_ARGUMENT, status_zero);
    assert_equal("FLASH STORAGE - READ EXCEEDED SIZE", POS_ERROR_INVALID_ARGUMENT, status_exceeded);
}

void test_flash_storage_read_invalid_address(void) {

    FlashStorage storage;
    unsigned char buffer[5];

    flash_storage_init(&storage);

    POSStatus status = flash_storage_read(&storage, FLASH_STORAGE_SIZE, buffer, sizeof(buffer));

    assert_equal("FLASH STORAGE - READ INVALID ADDRESS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_read_out_of_bounds(void) {

    FlashStorage storage;
    unsigned char buffer[5];

    flash_storage_init(&storage);

    POSStatus status = flash_storage_read(&storage, FLASH_STORAGE_SIZE - 2, buffer, sizeof(buffer));

    assert_equal("FLASH STORAGE - READ OUT OF BOUNDS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_write(void) {

    FlashStorage storage;
    unsigned char data = 0xAA;

    flash_storage_init(&storage);

    POSStatus status = flash_storage_write(&storage, 10, &data, 1);

    assert_equal("FLASH STORAGE - WRITE STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - WRITE DATA", 0xAA, storage.data[10]);
}

void test_flash_storage_write_multiple(void) {

    FlashStorage storage;

    unsigned char data[] = {0xAA, 0xBB, 0xCC};

    flash_storage_init(&storage);

    POSStatus status = flash_storage_write(&storage, 10, data, sizeof(data));

    assert_equal("FLASH STORAGE - WRITE MULTIPLE STATUS", POS_SUCCESS, status);

    int valid = (
        storage.data[10] == 0xAA &&
        storage.data[11] == 0xBB &&
        storage.data[12] == 0xCC
    );

    assert_equal("FLASH STORAGE - WRITE MULTIPLE DATA", 1, valid);
}

void test_flash_storage_write_invalid_bits(void) {

    FlashStorage storage;

    unsigned char first_data = 0xAA;
    unsigned char second_data = 0xFF;

    flash_storage_init(&storage);

    flash_storage_write(&storage, 10, &first_data, 1);

    POSStatus status = flash_storage_write(&storage, 10, &second_data, 1);

    assert_equal("FLASH STORAGE - WRITE INVALID BITS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_write_partial_failure(void) {

    FlashStorage storage;

    unsigned char initial_data[] = {0xAA, 0xBB, 0xCC};
    unsigned char new_data[] = {0x11, 0x22, 0xFF};

    flash_storage_init(&storage);

    flash_storage_write(&storage, 10, initial_data, sizeof(initial_data));

    POSStatus status = flash_storage_write(&storage, 10, new_data, sizeof(new_data));

    assert_equal("FLASH STORAGE - WRITE PARTIAL STATUS", POS_ERROR_INVALID_ARGUMENT, status);

    int unchanged = (
        storage.data[10] == 0xAA &&
        storage.data[11] == 0xBB &&
        storage.data[12] == 0xCC
    );

    assert_equal("FLASH STORAGE - WRITE PARTIAL DATA", 1, unchanged);
}

void test_flash_storage_write_null(void) {

    FlashStorage storage;
    unsigned char data = 0xAA;

    flash_storage_init(&storage);

    POSStatus status_storage = flash_storage_write(NULL, 0, &data, 1);

    POSStatus status_data = flash_storage_write(&storage, 0, NULL, 1);

    assert_equal("FLASH STORAGE - WRITE NULL STORAGE", POS_ERROR_NULL_POINTER, status_storage);

    assert_equal("FLASH STORAGE - WRITE NULL DATA", POS_ERROR_NULL_POINTER, status_data);
}

void test_flash_storage_write_invalid_size(void) {

    FlashStorage storage;

    unsigned char data[FLASH_STORAGE_MAX_OPERATION_SIZE + 1] = {0};

    flash_storage_init(&storage);

    POSStatus status_zero = flash_storage_write(&storage, 0, data, 0);

    POSStatus status_exceeded = flash_storage_write(&storage, 0, data, sizeof(data));

    assert_equal("FLASH STORAGE - WRITE ZERO SIZE", POS_ERROR_INVALID_ARGUMENT, status_zero);

    assert_equal("FLASH STORAGE - WRITE EXCEEDED SIZE", POS_ERROR_INVALID_ARGUMENT, status_exceeded);
}

void test_flash_storage_write_invalid_address(void) {

    FlashStorage storage;
    unsigned char data = 0xAA;

    flash_storage_init(&storage);

    POSStatus status = flash_storage_write(&storage, FLASH_STORAGE_SIZE, &data, 1);

    assert_equal("FLASH STORAGE - WRITE INVALID ADDRESS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_write_out_of_bounds(void) {

    FlashStorage storage;
    unsigned char data[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

    flash_storage_init(&storage);

    POSStatus status = flash_storage_write(&storage, FLASH_STORAGE_SIZE - 2, data, sizeof(data));

    assert_equal("FLASH STORAGE - WRITE OUT OF BOUNDS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_erase(void) {

    FlashStorage storage;
    unsigned char data = 0xAA;

    flash_storage_init(&storage);
    flash_storage_write(&storage, 10, &data, 1);

    POSStatus status = flash_storage_erase(&storage, 10, 1);

    assert_equal("FLASH STORAGE - ERASE STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - ERASE DATA", 0xFF, storage.data[10]);
}

void test_flash_storage_erase_multiple(void) {

    FlashStorage storage;
    unsigned char data[] = {0xAA, 0xBB, 0xCC};

    flash_storage_init(&storage);
    flash_storage_write(&storage, 10, data, sizeof(data));

    POSStatus status = flash_storage_erase(&storage, 10, sizeof(data));

    int erased = (
        storage.data[10] == 0xFF &&
        storage.data[11] == 0xFF &&
        storage.data[12] == 0xFF
    );

    assert_equal("FLASH STORAGE - ERASE MULTIPLE STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - ERASE MULTIPLE DATA", 1, erased);
}

void test_flash_storage_erase_preserve_data(void) {

    FlashStorage storage;
    unsigned char data[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

    flash_storage_init(&storage);
    flash_storage_write(&storage, 10, data, sizeof(data));

    POSStatus status = flash_storage_erase(&storage, 11, 3);

    int valid = (
        storage.data[10] == 0xAA &&
        storage.data[11] == 0xFF &&
        storage.data[12] == 0xFF &&
        storage.data[13] == 0xFF &&
        storage.data[14] == 0xEE
    );

    assert_equal("FLASH STORAGE - ERASE PRESERVE STATUS", POS_SUCCESS, status);
    assert_equal("FLASH STORAGE - ERASE PRESERVE DATA", 1, valid);
}

void test_flash_storage_erase_null(void) {

    POSStatus status = flash_storage_erase(NULL, 0, 1);

    assert_equal("FLASH STORAGE - ERASE NULL", POS_ERROR_NULL_POINTER, status);
}

void test_flash_storage_erase_invalid_size(void) {

    FlashStorage storage;

    flash_storage_init(&storage);

    POSStatus status_zero = flash_storage_erase(&storage, 0, 0);

    POSStatus status_exceeded = flash_storage_erase(&storage, 0, FLASH_STORAGE_MAX_OPERATION_SIZE + 1);

    assert_equal("FLASH STORAGE - ERASE ZERO SIZE", POS_ERROR_INVALID_ARGUMENT, status_zero);

    assert_equal("FLASH STORAGE - ERASE EXCEEDED SIZE", POS_ERROR_INVALID_ARGUMENT, status_exceeded);
}

void test_flash_storage_erase_invalid_address(void) {

    FlashStorage storage;

    flash_storage_init(&storage);

    POSStatus status = flash_storage_erase(&storage, FLASH_STORAGE_SIZE, 1);

    assert_equal("FLASH STORAGE - ERASE INVALID ADDRESS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_erase_out_of_bounds(void) {

    FlashStorage storage;

    flash_storage_init(&storage);

    POSStatus status = flash_storage_erase(&storage, FLASH_STORAGE_SIZE - 2, 5);

    assert_equal("FLASH STORAGE - ERASE OUT OF BOUNDS", POS_ERROR_INVALID_ARGUMENT, status);
}

void test_flash_storage_write_after_erase(void) {

    FlashStorage storage;

    unsigned char first_data = 0xAA;
    unsigned char second_data = 0xFF;

    flash_storage_init(&storage);

    flash_storage_write(&storage, 10, &first_data, 1);

    flash_storage_erase(&storage, 10, 1);

    POSStatus status = flash_storage_write(&storage, 10, &second_data, 1);

    assert_equal("FLASH STORAGE - WRITE AFTER ERASE STATUS", POS_SUCCESS, status);

    assert_equal("FLASH STORAGE - WRITE AFTER ERASE DATA", 0xFF, storage.data[10]);
}
