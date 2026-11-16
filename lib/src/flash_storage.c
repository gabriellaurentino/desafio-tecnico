/**
 * @file flash_storage.c
 * @author Gabriel Laurentino
 * @brief Implementação do armazenamento Flash simulado para persistência de dados.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "flash_storage.h"
#include <string.h>


POSStatus flash_storage_init(FlashStorage *storage) {

    if (storage == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    memset(storage->data, 0xFF, FLASH_STORAGE_SIZE);

    return POS_SUCCESS;
}

POSStatus flash_storage_read(const FlashStorage *storage, size_t address, unsigned char *buffer, size_t length) {
    
    if (storage == NULL || buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (length == 0 || length > FLASH_STORAGE_MAX_OPERATION_SIZE) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    if (address >= FLASH_STORAGE_SIZE || length > FLASH_STORAGE_SIZE - address) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    memcpy(buffer, &storage->data[address], length);

    return POS_SUCCESS;
}

POSStatus flash_storage_write( FlashStorage *storage, size_t address, const unsigned char *data, size_t length) {
    
    if (storage == NULL || data == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (length == 0 || length > FLASH_STORAGE_MAX_OPERATION_SIZE) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    if (address >= FLASH_STORAGE_SIZE || length > FLASH_STORAGE_SIZE - address) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    for (size_t i = 0; i < length; i++) {
        if ((storage->data[address + i] & data[i]) != data[i]) {
            return POS_ERROR_INVALID_ARGUMENT;
        }
    }

    for (size_t i = 0; i < length; i++) {
        storage->data[address + i] &= data[i];
    }

    return POS_SUCCESS;
}

POSStatus flash_storage_erase( FlashStorage *storage, size_t address, size_t length) {

    if (storage == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (length == 0 || length > FLASH_STORAGE_MAX_OPERATION_SIZE) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    if (address >= FLASH_STORAGE_SIZE || length > FLASH_STORAGE_SIZE - address) {
        return POS_ERROR_INVALID_ARGUMENT;
    } 

    memset(&storage->data[address], 0xFF, length);

    return POS_SUCCESS;
}