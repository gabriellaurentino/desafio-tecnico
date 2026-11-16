/**
 * @file log.c
 * @author Gabriel Laurentino
 * @brief Implementação do registro e recuperação do histórico de transações POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "log.h"

#include <string.h>

#include "string_utils.h"
#include "utils.h"


POSStatus log_init(Log *log, FlashStorage *storage) {

    if (log == NULL || storage == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    log->storage = storage;
    log->write_address = 0;
    log->read_address = 0;

    return POS_SUCCESS;
}

static POSStatus mask_pan(const char *pan, char *masked_pan, size_t buffer_size) {

    if (pan == NULL || masked_pan == NULL) {
        return POS_ERROR_NULL_POINTER;
    }
    
    size_t pan_size = bounded_strlen(pan, 20);

    if (pan_size < 13 || pan_size > 19 || buffer_size < pan_size + 1) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < pan_size; i++) {
        if (pan[i] < '0' || pan[i] > '9') {
            return POS_ERROR_INVALID_FORMAT;
        }
    }

    memcpy(masked_pan, pan, 6);

    size_t masked_size = pan_size - 10;

    memset(masked_pan + 6, '*', masked_size);

    memcpy(masked_pan + pan_size - 4, pan + pan_size - 4, 4);

    masked_pan[pan_size] = '\0';

    return POS_SUCCESS;
}

static POSStatus validate_masked_pan(const char *pan) {

    if (pan == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    size_t length = bounded_strlen(pan, LOG_PAN_SIZE+1);

    if (length < 13 || length > 19) {
        return POS_ERROR_INVALID_FORMAT;
    }

    for (size_t i = 0; i < length; i++) {

        if (i < 6 || i >= length - 4) {

            if (pan[i] < '0' || pan[i] > '9') {
                return POS_ERROR_INVALID_FORMAT;
            }

        } else if (pan[i] != '*') {
            return POS_ERROR_INVALID_FORMAT;
        }
    }

    return POS_SUCCESS;
}

static void xor_crypt(unsigned char *data, size_t length) {
 
    for (size_t i = 0; i < length; i++) {
        data[i] ^= LOG_XOR_KEY;
    }
}

static POSStatus validate_response_code(LogTransactionStatus status, const char *response_code) {
    
    if (response_code == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (response_code[2] != '\0') {
        return POS_ERROR_INVALID_FORMAT;
    }

    if (status == LOG_TRANSACTION_TIMEOUT) {
        return strcmp(response_code, "TO") == 0 ? POS_SUCCESS : POS_ERROR_INVALID_FORMAT;
    }

    if (status == LOG_TRANSACTION_REVERSED) {
        return strcmp(response_code, "RV") == 0 ? POS_SUCCESS : POS_ERROR_INVALID_FORMAT;
    }

    if (status == LOG_TRANSACTION_APPROVED){
        return strcmp(response_code, "00") == 0 ? POS_SUCCESS : POS_ERROR_INVALID_FORMAT;
    } 

    if (status == LOG_TRANSACTION_DECLINED) {

        if (response_code[0] < '0' || response_code[0] > '9' || response_code[1] < '0' || response_code[1] > '9') {
            return POS_ERROR_INVALID_FORMAT;
        }

        return strcmp(response_code, "00") != 0 ? POS_SUCCESS : POS_ERROR_INVALID_FORMAT;
    }

    return POS_ERROR_INVALID_ARGUMENT;
}

static POSStatus serialize_record(const LogRecord *record, unsigned char *buffer, size_t buffer_size) {

    if (record == NULL || buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (buffer_size < LOG_RECORD_SIZE) {
        return POS_ERROR_INVALID_LENGTH;
    }

    char masked_pan[LOG_PAN_SIZE + 1];

    POSStatus status = mask_pan(record->pan, masked_pan, sizeof(masked_pan));

    if (status != POS_SUCCESS) {
        return status;
    }

    if (record->status < LOG_TRANSACTION_APPROVED || record->status > LOG_TRANSACTION_REVERSED) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    status = validate_response_code(record->status, record->response_code);

    if (status != POS_SUCCESS) {
        return status;
    }

    size_t offset = 0;

    write_uint32_be(&buffer[offset], record->timestamp);
    offset += LOG_TIMESTAMP_SIZE;

    write_uint32_be(&buffer[offset], record->transaction_id);
    offset += LOG_TRANSACTION_ID_SIZE;

    write_uint32_be(&buffer[offset], record->amount);
    offset += LOG_AMOUNT_SIZE;

    buffer[offset++] = (unsigned char)record->status;

    memset(&buffer[offset], 0, LOG_PAN_SIZE);
    memcpy(&buffer[offset], masked_pan, strlen(masked_pan));
    offset += LOG_PAN_SIZE;

    memcpy(&buffer[offset], record->response_code, LOG_RESPONSE_CODE_SIZE);
    offset += LOG_RESPONSE_CODE_SIZE;

    if (offset != LOG_RECORD_SIZE) {
        return POS_ERROR_INVALID_LENGTH;
    }

    return POS_SUCCESS;
}

POSStatus log_write(Log *log, const LogRecord *record) {

    if (log == NULL || log->storage == NULL || record == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    unsigned char serialized_record[LOG_RECORD_SIZE];

    POSStatus status = serialize_record(record, serialized_record, sizeof(serialized_record));

    if (status != POS_SUCCESS) {
        return status;
    }

    unsigned char storage_buffer[LOG_STORAGE_RECORD_SIZE];

    storage_buffer[0] = (unsigned char)((LOG_RECORD_SIZE >> 8) & 0xFF);
    storage_buffer[1] = (unsigned char)(LOG_RECORD_SIZE & 0xFF);

    memcpy(&storage_buffer[LOG_HEADER_SIZE], serialized_record, sizeof(serialized_record));

    xor_crypt(&storage_buffer[LOG_HEADER_SIZE], sizeof(serialized_record));

    if (log->write_address > FLASH_STORAGE_SIZE || LOG_STORAGE_RECORD_SIZE > FLASH_STORAGE_SIZE - log->write_address) {
        return POS_ERROR_LOG_STORAGE_FULL;
    }

    status = flash_storage_write(log->storage, log->write_address, storage_buffer, LOG_STORAGE_RECORD_SIZE);

    if (status != POS_SUCCESS) {
        return status;
    }

    log->write_address += LOG_STORAGE_RECORD_SIZE;

    return POS_SUCCESS;
}

static POSStatus deserialize_record(const unsigned char *buffer, size_t buffer_size, LogRecord *record) {

    if (buffer == NULL || record == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (buffer_size < LOG_RECORD_SIZE) {
        return POS_ERROR_INVALID_LENGTH;
    }

    LogRecord temp_record = {0};

    size_t offset = 0;

    temp_record.timestamp = read_uint32_be(&buffer[offset]);
    offset += LOG_TIMESTAMP_SIZE;

    temp_record.transaction_id = read_uint32_be(&buffer[offset]);
    offset += LOG_TRANSACTION_ID_SIZE;

    temp_record.amount = read_uint32_be(&buffer[offset]);
    offset += LOG_AMOUNT_SIZE;

    if (buffer[offset] > LOG_TRANSACTION_REVERSED) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    temp_record.status = (LogTransactionStatus)buffer[offset++];

    memcpy(temp_record.pan, &buffer[offset], LOG_PAN_SIZE);
    temp_record.pan[LOG_PAN_SIZE] = '\0';
    offset += LOG_PAN_SIZE;

    POSStatus status = validate_masked_pan(temp_record.pan);
    
    if (status != POS_SUCCESS) {
        return status;
    }

    memcpy(temp_record.response_code, &buffer[offset], LOG_RESPONSE_CODE_SIZE);
    temp_record.response_code[LOG_RESPONSE_CODE_SIZE] = '\0';
    offset += LOG_RESPONSE_CODE_SIZE;

    if (offset != LOG_RECORD_SIZE) {
        return POS_ERROR_INVALID_LENGTH;
    }
    
    status = validate_response_code(temp_record.status, temp_record.response_code);

    if (status != POS_SUCCESS) {
        return status;
    }

    *record = temp_record;

    return POS_SUCCESS;

}

POSStatus log_read(Log *log, LogRecord *record) {
    
    if (log == NULL || log->storage == NULL || record == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (log->write_address == 0) {
        return POS_ERROR_LOG_NO_RECORD;
    }

    if (log->write_address > FLASH_STORAGE_SIZE || log->read_address > log->write_address) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    size_t read_address = log->read_address;

    if (read_address == log->write_address) {
        return POS_ERROR_LOG_NO_RECORD;
    }

    if (LOG_HEADER_SIZE > log->write_address - read_address) {
        return POS_ERROR_INVALID_LENGTH;
    }

    unsigned char header[LOG_HEADER_SIZE];

    POSStatus status = flash_storage_read(log->storage, read_address, header, LOG_HEADER_SIZE);

    if (status != POS_SUCCESS) {
        return status;
    }

    size_t record_size = ((size_t)header[0] << 8) | header[1];

    if (record_size != LOG_RECORD_SIZE) {
        return POS_ERROR_INVALID_LENGTH;
    }
    
    size_t storage_record_size = record_size + LOG_HEADER_SIZE;

    if (storage_record_size > log->write_address - read_address) {
        return POS_ERROR_INVALID_LENGTH;
    }

    unsigned char buffer[LOG_RECORD_SIZE];

    status = flash_storage_read(log->storage, read_address + LOG_HEADER_SIZE, buffer, LOG_RECORD_SIZE);

    if (status != POS_SUCCESS) {
        return status;
    }

    xor_crypt(buffer, LOG_RECORD_SIZE);

    LogRecord temp_record = {0};

    status = deserialize_record(buffer, sizeof(buffer), &temp_record);

    if (status != POS_SUCCESS) {
        return status;
    }

    *record = temp_record;
    log->read_address = read_address + storage_record_size;

    return POS_SUCCESS;
}

POSStatus log_reset_read(Log *log) {
    
    if (log == NULL || log->storage == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    log->read_address = 0;

    return POS_SUCCESS;
}