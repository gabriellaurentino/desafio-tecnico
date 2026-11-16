/**
 * @file iso8583.c
 * @author Gabriel Laurentino
 * @brief Implementação de construção, serialização e parsing de mensagens ISO8583.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "iso8583.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "string_utils.h"
#include "luhn.h"

#define ISO8583_FIELD_COUNT 11


typedef enum {
    ISO8583_FIXED,
    ISO8583_LLVAR,
    ISO8583_LLLVAR
} ISO8583FieldFormat;

typedef struct {
    int field;
    const char *value;
    size_t max_size;
    size_t buffer_size;
    ISO8583FieldFormat format;
} ISO8583Field;

typedef struct {
    int field;
    char *value;
    size_t max_size;
    ISO8583FieldFormat format;
} ISO8583ParseField;

static POSStatus append_bytes(ISO8583Buffer *buffer, const void *data, size_t size) {
    
    if (buffer == NULL || data == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (buffer->length > sizeof(buffer->data) || size > (sizeof(buffer->data) - buffer->length)) {
        return POS_ERROR_INVALID_LENGTH;
    }

    memcpy(buffer->data + buffer->length, data, size);

    buffer->length += size;

    return POS_SUCCESS;
}

POSStatus iso8583_set_bitmap_bit(ISO8583Buffer *buffer, int field) {

    if (buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (field < 2 || field > 128) {
        return POS_ERROR_INVALID_LENGTH;
    }

    if (field > 64) {
        buffer->primary_bitmap[0] |= (1 << 7);
        
        int adj_field = field - 65;
        int byte_index = adj_field / 8;
        int bit_index = 7 - (adj_field % 8);

        buffer->secondary_bitmap[byte_index] |= (unsigned char)(1 << bit_index);
    } else {
        int adj_field = field - 1;
        int byte_index = adj_field / 8;
        int bit_index = 7 - (adj_field % 8);

        buffer->primary_bitmap[byte_index] |= (unsigned char)(1 << bit_index);
    }
    return POS_SUCCESS;
}

static void iso8583_get_fields(const ISO8583Message *message, ISO8583Field fields[ISO8583_FIELD_COUNT]) {

    fields[0] = (ISO8583Field){ISO8583_FIELD_PAN, message->pan, ISO8583_SIZE_PAN, sizeof(message->pan), ISO8583_LLVAR};
    fields[1] = (ISO8583Field){ISO8583_FIELD_PROCESSING_CODE, message->processing_code, ISO8583_SIZE_PROCESSING_CODE, sizeof(message->processing_code), ISO8583_FIXED};
    fields[2] = (ISO8583Field){ISO8583_FIELD_AMOUNT, message->amount, ISO8583_SIZE_AMOUNT, sizeof(message->amount), ISO8583_FIXED};
    fields[3] = (ISO8583Field){ISO8583_FIELD_TRANSMISSION_DATE, message->transmission_date, ISO8583_SIZE_TRANSMISSION_DATE, sizeof(message->transmission_date), ISO8583_FIXED};
    fields[4] = (ISO8583Field){ISO8583_FIELD_STAN, message->stan, ISO8583_SIZE_STAN, sizeof(message->stan), ISO8583_FIXED};
    fields[5] = (ISO8583Field){ISO8583_FIELD_LOCAL_TIME, message->local_time, ISO8583_SIZE_LOCAL_TIME, sizeof(message->local_time), ISO8583_FIXED};
    fields[6] = (ISO8583Field){ISO8583_FIELD_RETRIEVAL_REFERENCE, message->retrieval_reference, ISO8583_SIZE_RETRIEVAL_REFERENCE, sizeof(message->retrieval_reference), ISO8583_FIXED};
    fields[7] = (ISO8583Field){ISO8583_FIELD_RESPONSE_CODE, message->response_code, ISO8583_SIZE_RESPONSE_CODE, sizeof(message->response_code), ISO8583_FIXED};
    fields[8] = (ISO8583Field){ISO8583_FIELD_TERMINAL_ID, message->terminal_id, ISO8583_SIZE_TERMINAL_ID, sizeof(message->terminal_id), ISO8583_FIXED};
    fields[9] = (ISO8583Field){ISO8583_FIELD_CURRENCY_CODE, message->currency_code, ISO8583_SIZE_CURRENCY_CODE, sizeof(message->currency_code), ISO8583_FIXED};
    fields[10] = (ISO8583Field){ISO8583_FIELD_MAC, message->mac, ISO8583_SIZE_MAC, sizeof(message->mac), ISO8583_FIXED};

}

POSStatus iso8583_build_bitmap(const ISO8583Message *message, ISO8583Buffer *buffer) {
    
    if (message == NULL || buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    ISO8583Buffer temp_buffer = {0};

    ISO8583Field fields[ISO8583_FIELD_COUNT];

    iso8583_get_fields(message, fields);

    for (size_t i = 0; i < ISO8583_FIELD_COUNT; i++) {

        size_t value_size = bounded_strlen(fields[i].value, fields[i].buffer_size);

        if (value_size > 0) {

            if (fields[i].field == ISO8583_FIELD_PAN) {

                POSStatus status = validate_card_number(fields[i].value, fields[i].buffer_size);

                if (status != POS_SUCCESS) {
                    return status;
                }
                
            } else if (value_size > fields[i].max_size) {
                return POS_ERROR_INVALID_FIELD_LENGTH;
            }

            if (fields[i].format == ISO8583_FIXED && value_size != fields[i].max_size) {
                return POS_ERROR_INVALID_FIELD_LENGTH;
            } 

            POSStatus status = iso8583_set_bitmap_bit(&temp_buffer, fields[i].field);
            if (status != POS_SUCCESS) {
                return status;
            }
        }
    }

    memcpy(buffer->primary_bitmap, temp_buffer.primary_bitmap, sizeof(buffer->primary_bitmap));
    memcpy(buffer->secondary_bitmap, temp_buffer.secondary_bitmap, sizeof(buffer->secondary_bitmap));

    return POS_SUCCESS;

}

POSStatus iso8583_build(const ISO8583Message *message, ISO8583Buffer *buffer) {
    
    if (message == NULL || buffer == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (message->mti[4] != '\0' || (strcmp(message->mti, "0200") != 0 && strcmp(message->mti, "0210") != 0)) {
        return POS_ERROR_INVALID_MTI;
    }

    ISO8583Buffer temp_buffer = {0};

    POSStatus status = iso8583_build_bitmap(message, &temp_buffer);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = append_bytes(&temp_buffer, message->mti, 4);

    if (status != POS_SUCCESS) {
        return status;
    }

    status = append_bytes(&temp_buffer, temp_buffer.primary_bitmap, 8);

    if (status != POS_SUCCESS) {
        return status;
    }

    if ((temp_buffer.primary_bitmap[0] & 0x80) != 0) {

        status = append_bytes(&temp_buffer, temp_buffer.secondary_bitmap, 8);
        
        if (status != POS_SUCCESS) {
            return status;
        }
    }
    
    ISO8583Field fields[ISO8583_FIELD_COUNT];

    iso8583_get_fields(message, fields);

    for (size_t i = 0; i < ISO8583_FIELD_COUNT; i++) {

        size_t value_size = bounded_strlen(fields[i].value, fields[i].max_size + 1);

        if (value_size > fields[i].max_size){
            return POS_ERROR_INVALID_FIELD_LENGTH;
        }

        if (value_size > 0 &&  fields[i].format == ISO8583_FIXED && value_size != fields[i].max_size) {
            return POS_ERROR_INVALID_FIELD_LENGTH;
        }

        if (value_size == 0) {
            continue;
        }

        if (fields[i].format == ISO8583_FIXED) {
           
            status = append_bytes(&temp_buffer, fields[i].value, fields[i].max_size);
            
            if (status != POS_SUCCESS) {
                return status;
            }
            
        } else if (fields[i].format == ISO8583_LLVAR) {

            char prefix[3];

            snprintf(prefix, sizeof(prefix), "%02zu", value_size);

            status = append_bytes(&temp_buffer, prefix, 2);

            if (status != POS_SUCCESS) {
                return status;
            }

            status = append_bytes(&temp_buffer, fields[i].value, value_size);

            if (status != POS_SUCCESS) {
                return status;
            }

        } else {
            return POS_ERROR_INVALID_FIELD_FORMAT;
        }

    }

    *buffer = temp_buffer;

    return POS_SUCCESS;
}

static int is_bit_set(const unsigned char *bitmap, int bit) {

    if (bitmap == NULL || bit < 1 || bit > 64)  {
        return 0;
    }
        
    int byte_index = (bit - 1) / 8;
    int bit_offset = (bit - 1) % 8;

    return (bitmap[byte_index] & (0x80 >> bit_offset)) != 0;
}

static int iso8583_is_field_set(const unsigned char *bitmap_primary, const unsigned char *bitmap_secondary, int field) {
    if (field <= 64) {
        return is_bit_set(bitmap_primary, field);
    }
    return is_bit_set(bitmap_secondary, field - 64);
}

static POSStatus iso8583_validate_bitmap(const unsigned char *bitmap_primary, const unsigned char *bitmap_secondary) {
    const int supported_fields[] = {
        ISO8583_FIELD_PAN,
        ISO8583_FIELD_PROCESSING_CODE,
        ISO8583_FIELD_AMOUNT,
        ISO8583_FIELD_TRANSMISSION_DATE,
        ISO8583_FIELD_STAN,
        ISO8583_FIELD_LOCAL_TIME,
        ISO8583_FIELD_RETRIEVAL_REFERENCE,
        ISO8583_FIELD_RESPONSE_CODE,
        ISO8583_FIELD_TERMINAL_ID,
        ISO8583_FIELD_CURRENCY_CODE,
        ISO8583_FIELD_MAC
    };

    const size_t total_fields = sizeof(supported_fields) / sizeof(supported_fields[0]);

    for (int field = 2; field <= 128; field++) {

        if (!iso8583_is_field_set(bitmap_primary, bitmap_secondary, field)) {
            continue;
        }

        int supported = 0;

        for (size_t i = 0; i < total_fields; i++) {
            if (supported_fields[i] == field) {
                supported = 1;
                break;
            }
        }

        if (!supported) {
            return POS_ERROR_INVALID_FIELD;
        }
    }

    return POS_SUCCESS;
}

static int has_remaining_data(size_t length, size_t offset, size_t required) {
    return offset <= length && required <= length - offset;
}

POSStatus iso8583_parse(const unsigned char *data, size_t length, ISO8583Message *message) {
    
    if (data == NULL || message == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (length < 12) {
        return POS_ERROR_INVALID_FIELD_LENGTH;
    }

    size_t offset = 0;

    ISO8583Message temp_message = {0};

    memcpy(temp_message.mti, data + offset, 4);
    temp_message.mti[4] = '\0';
    offset += 4;

    if (strcmp(temp_message.mti, "0200") != 0 && strcmp(temp_message.mti, "0210") != 0) {
        return POS_ERROR_INVALID_MTI;
    }

    unsigned char bitmap_primary[8];
    unsigned char bitmap_secondary[8] = {0};
    
    if (!has_remaining_data(length, offset, 8)) {
        return POS_ERROR_INVALID_FIELD_LENGTH;
    }

    memcpy(bitmap_primary, data + offset, 8);
    offset += 8;

    if (is_bit_set(bitmap_primary, 1)) {

        if (!has_remaining_data(length, offset, 8)) {
            return POS_ERROR_INVALID_FIELD_LENGTH;
        }

        memcpy(bitmap_secondary, data + offset, 8);
        offset += 8;

    }

    POSStatus status = iso8583_validate_bitmap(bitmap_primary, bitmap_secondary);

    if (status != POS_SUCCESS) {
        return status;
    }

    ISO8583ParseField fields[] = {
        {ISO8583_FIELD_PAN, temp_message.pan, ISO8583_SIZE_PAN, ISO8583_LLVAR},
        {ISO8583_FIELD_PROCESSING_CODE, temp_message.processing_code, ISO8583_SIZE_PROCESSING_CODE, ISO8583_FIXED},
        {ISO8583_FIELD_AMOUNT, temp_message.amount, ISO8583_SIZE_AMOUNT, ISO8583_FIXED},
        {ISO8583_FIELD_TRANSMISSION_DATE, temp_message.transmission_date, ISO8583_SIZE_TRANSMISSION_DATE, ISO8583_FIXED},
        {ISO8583_FIELD_STAN, temp_message.stan, ISO8583_SIZE_STAN, ISO8583_FIXED},
        {ISO8583_FIELD_LOCAL_TIME, temp_message.local_time, ISO8583_SIZE_LOCAL_TIME, ISO8583_FIXED},
        {ISO8583_FIELD_RETRIEVAL_REFERENCE, temp_message.retrieval_reference, ISO8583_SIZE_RETRIEVAL_REFERENCE, ISO8583_FIXED},
        {ISO8583_FIELD_RESPONSE_CODE, temp_message.response_code, ISO8583_SIZE_RESPONSE_CODE, ISO8583_FIXED},
        {ISO8583_FIELD_TERMINAL_ID, temp_message.terminal_id, ISO8583_SIZE_TERMINAL_ID, ISO8583_FIXED},
        {ISO8583_FIELD_CURRENCY_CODE, temp_message.currency_code, ISO8583_SIZE_CURRENCY_CODE, ISO8583_FIXED},
        {ISO8583_FIELD_MAC, temp_message.mac, ISO8583_SIZE_MAC, ISO8583_FIXED}
    };

    size_t total_fields = sizeof(fields) / sizeof(fields[0]);

    for (size_t i = 0; i < total_fields; i++) {

        ISO8583ParseField *field = &fields[i];

        if (!iso8583_is_field_set(bitmap_primary, bitmap_secondary, field->field)) {
            continue;
        }

        size_t value_size = field->max_size;

        if (field->format == ISO8583_LLVAR) {

            if (!has_remaining_data(length, offset, 2)) {
                return POS_ERROR_INVALID_FIELD_LENGTH;
            }

            if (!isdigit((unsigned char)data[offset]) || !isdigit((unsigned char)data[offset + 1])) {
                return POS_ERROR_INVALID_FORMAT;
            }

            value_size = (size_t)((data[offset] - '0') * 10 + (data[offset + 1] - '0'));

            offset += 2;

            if (value_size == 0 || value_size > field->max_size) {
                return POS_ERROR_INVALID_FIELD_LENGTH;
            }
        }

        if (!has_remaining_data(length, offset, value_size)) {
            return POS_ERROR_INVALID_FIELD_LENGTH;
        }

        memcpy(field->value, data + offset, value_size);

        field->value[value_size] = '\0';


        offset += value_size;

        if (field->field == ISO8583_FIELD_PAN) {
            status = validate_card_number(field->value, field->max_size + 1);

            if (status != POS_SUCCESS) {
                return status;
            }
        }
    }

    if (offset != length) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    *message = temp_message;

    return POS_SUCCESS;
}
