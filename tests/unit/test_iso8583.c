#include "iso8583.h"
#include "test_utils.h"
#include <string.h>


static void fill_valid_message(ISO8583Message *message) {
    strcpy(message->pan, "4111111111111111");
    strcpy(message->processing_code, "000000");
    strcpy(message->amount, "000000015000");
    strcpy(message->transmission_date, "1112123456");
    strcpy(message->stan, "123456");
    strcpy(message->local_time, "123456");
    strcpy(message->retrieval_reference, "123456789012");
    strcpy(message->terminal_id, "TERM0001");
    strcpy(message->currency_code, "986");
    strcpy(message->mac, "0123456789ABCDEF");
}


void test_set_bitmap_field_2(void) {

    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_PAN);

    assert_equal("ISO8583 - BITMAP BIT - Set bitmap field 2", POS_SUCCESS, result);

    assert_equal("ISO8583 - BITMAP BIT - Bitmap field 2 should be activated", 0x40, buffer.primary_bitmap[0]);

}

void test_set_bitmap_field_3(void) {

    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_PROCESSING_CODE);

    assert_equal("ISO8583 - BITMAP BIT - Set bitmap field 3", POS_SUCCESS, result);

    assert_equal("ISO8583 - BITMAP BIT - Bitmap field 3 should be activated", 0x20, buffer.primary_bitmap[0]);

}

void test_set_bitmap_field_64(void) {

    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, 64);

    assert_equal("ISO8583 - BITMAP BIT - Set bitmap field 64", POS_SUCCESS, result);

    assert_equal("ISO8583 - BITMAP BIT - Bitmap field 64 should be activated", 0x01, buffer.primary_bitmap[7]);

}

void test_set_bitmap_field_65(void) {

    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, 65);

    assert_equal("ISO8583 - BITMAP BIT - Set bitmap field 65", POS_SUCCESS, result);

    assert_equal("ISO8583 - BITMAP BIT - Bitmap field 65 should be activated", 0x80, buffer.secondary_bitmap[0]);
    assert_equal("ISO8583 - BITMAP BIT - Secondary Bitmap indicator", 0x80, buffer.primary_bitmap[0]);

}

void test_set_bitmap_field_128(void) {
    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_MAC);

    assert_equal("ISO8583 - BITMAP BIT - Set bitmap field 128", POS_SUCCESS, result);

    assert_equal("ISO8583 - BITMAP BIT - Bitmap field 128 should be activated", 0x01, buffer.secondary_bitmap[7]);
    assert_equal("ISO8583 - BITMAP BIT - Secondary Bitmap indicator", 0x80, buffer.primary_bitmap[0]);
}

void test_set_bitmap_invalid_field_1(void) {
    ISO8583Buffer buffer = {0};

    int result = iso8583_set_bitmap_bit(&buffer, 1);

    assert_equal("ISO8583 - BITMAP BIT - Invalid field 1", POS_ERROR_INVALID_LENGTH, result);   
}

void test_set_bitmap_invalid_field_129(void) {
    ISO8583Buffer buffer = {0};
    
    int result = iso8583_set_bitmap_bit(&buffer, 129);
    
    assert_equal("ISO8583 - BITMAP BIT - Invalid field 129", POS_ERROR_INVALID_LENGTH, result);
    assert_equal("ISO8583 - BITMAP BIT - Primary bitmap unchanged", 0, buffer.primary_bitmap[0]);
    assert_equal("ISO8583 - BITMAP BIT - Secondary bitmap unchanged", 0, buffer.secondary_bitmap[0]);
}

void test_set_bitmap_null_buffer(void) {
    int result = iso8583_set_bitmap_bit(NULL, 2);
    
    assert_equal("ISO8583 - BITMAP BIT - Null bitmap buffer", POS_ERROR_NULL_POINTER, result);
}

void test_set_bitmap_same_field_twice(void) {
    ISO8583Buffer buffer = {0};
    
    iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_PAN);
    iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_PAN);
    
    assert_equal("ISO8583 - BITMAP BIT - Repeated field 2 activation", 0x40, buffer.primary_bitmap[0]);
}

void test_set_bitmap_preserves_other_bits(void) {
    ISO8583Buffer buffer = {0};

    iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_PAN);
    iso8583_set_bitmap_bit(&buffer, ISO8583_FIELD_MAC);

    assert_equal("ISO8583 - BITMAP BIT - Primary field 2 preserved", 0xC0, buffer.primary_bitmap[0]);
    assert_equal("ISO8583 - BITMAP BIT - Secondary field 128 activated", 0x01, buffer.secondary_bitmap[7]);

}

void test_build_bitmap_all_fields(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    fill_valid_message(&message);

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap with all fields", 0, result);

    assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap byte 0", 0xF2, buffer.primary_bitmap[0]);
    assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap byte 1", 0x30, buffer.primary_bitmap[1]);
    assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap byte 4", 0x08, buffer.primary_bitmap[4]);
    assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap byte 5", 0x80, buffer.primary_bitmap[5]);
    assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap byte 6", 0x80, buffer.primary_bitmap[6]);

    assert_equal("ISO8583 - BUILD BITMAP - Secondary bitmap byte 7", 0x01, buffer.secondary_bitmap[7]);

}

void test_build_bitmap_only_pan(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "4111111111111111");

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap only PAN", POS_SUCCESS, result);
    assert_equal("ISO8583 - BUILD BITMAP - PAN field activated", 0x40, buffer.primary_bitmap[0]);
    assert_equal("ISO8583 - BUILD BITMAP - Secondary bitmap remains empty", 0x00, buffer.secondary_bitmap[0]);

}

void test_build_bitmap_only_mac(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.mac, "0123456789ABCDEF");

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap only MAC", POS_SUCCESS, result);
    assert_equal("ISO8583 - BUILD BITMAP - Secondary bitmap indicator", 0x80, buffer.primary_bitmap[0]);
    assert_equal("ISO8583 - BUILD BITMAP - MAC field activated", 0x01, buffer.secondary_bitmap[7]);

}

void test_build_bitmap_empty_message(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer;

    memset(&buffer, 0xFF, sizeof(buffer));

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap empty message", POS_SUCCESS, result);

    for (size_t i = 0; i < 8; i++) {
        assert_equal("ISO8583 - BUILD BITMAP - Primary bitmap cleared", 0x00, buffer.primary_bitmap[i]);
        assert_equal("ISO8583 - BUILD BITMAP - Secondary bitmap cleared", 0x00, buffer.secondary_bitmap[i]);
    }

}

void test_build_bitmap_invalid_pan(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "4111111111111112");

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Invalid PAN", POS_ERROR_INVALID_PAN, result);

}

void test_build_bitmap_fixed_field_too_short(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.processing_code, "00000");

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Fixed field too short", POS_ERROR_INVALID_FIELD_LENGTH, result);

}

void test_build_bitmap_pan_variable_size(void) {

    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "4111111111111111");

    int result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - LLVAR PAN with variable size", POS_SUCCESS, result);
    assert_equal("ISO8583 - BUILD BITMAP - PAN field activated", 0x40, buffer.primary_bitmap[0]);

}

void test_build_bitmap_null_message(void) {

    ISO8583Buffer buffer = {0};

    int result = iso8583_build_bitmap(NULL, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Null message", POS_ERROR_NULL_POINTER, result);

}

void test_build_bitmap_null_buffer(void) {

    ISO8583Message message = {0};

    int result = iso8583_build_bitmap(&message, NULL);

    assert_equal("ISO8583 - BUILD BITMAP - Null buffer", POS_ERROR_NULL_POINTER, result);

}

void test_build_bitmap_preserves_original_on_error(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    memset(buffer.primary_bitmap, 0xAA, sizeof(buffer.primary_bitmap));
    memset(buffer.secondary_bitmap, 0xBB, sizeof(buffer.secondary_bitmap));

    strcpy(message.processing_code, "000000");
    strcpy(message.amount, "15000");

    POSStatus result = iso8583_build_bitmap(&message, &buffer);

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap atomicity error", POS_ERROR_INVALID_FIELD_LENGTH, result);

    int primary_preserved = 1;

    for (size_t i = 0; i < sizeof(buffer.primary_bitmap); i++) {
        if (buffer.primary_bitmap[i] != 0xAA) {
            primary_preserved = 0;
            break;
        }
    }

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap preserves primary bitmap", 1, primary_preserved);

    // Verifica se o bitmap secundário foi preservado
    int secondary_preserved = 1;

    for (size_t i = 0; i < sizeof(buffer.secondary_bitmap); i++) {
        if (buffer.secondary_bitmap[i] != 0xBB) {
            secondary_preserved = 0;
            break;
        }
    }

    assert_equal("ISO8583 - BUILD BITMAP - Build bitmap preserves secondary bitmap", 1,secondary_preserved);
}

void test_build_bitmap_pan_too_short(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "41111111111");

    assert_equal("ISO8583 - BUILD BITMAP - PAN too short", POS_ERROR_INVALID_LENGTH, iso8583_build_bitmap(&message, &buffer));
}

void test_build_bitmap_pan_too_long(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "41111111111111111111");

    assert_equal("ISO8583 - BUILD BITMAP - PAN too long", POS_ERROR_INVALID_FORMAT, iso8583_build_bitmap(&message, &buffer));
}

void test_build_bitmap_pan_with_letters(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.pan, "41111111111111AB");

    assert_equal("ISO8583 - BUILD BITMAP - PAN with letters", POS_ERROR_INVALID_FORMAT, iso8583_build_bitmap(&message, &buffer));
}

void test_build_bitmap_processing_code_too_long(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.processing_code, "0000000");

    assert_equal("ISO8583 - BUILD BITMAP - Processing code too long", POS_ERROR_INVALID_FIELD_LENGTH, iso8583_build_bitmap(&message, &buffer));
}

void test_build_bitmap_amount_too_short(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.amount, "00000001500");

    assert_equal("ISO8583 - BUILD BITMAP - Amount too short", POS_ERROR_INVALID_FIELD_LENGTH, iso8583_build_bitmap(&message, &buffer));
}

void test_build_bitmap_amount_too_long(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.amount, "0000000015000");

    assert_equal("ISO8583 - BUILD BITMAP - Amount too long", POS_ERROR_INVALID_FIELD_LENGTH, iso8583_build_bitmap(&message, &buffer));
}

void test_build_basic_message(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.mti, "0200");
    strcpy(message.pan, "4111111111111111");
    strcpy(message.processing_code, "000000");
    strcpy(message.amount, "000000015000");

    int result = iso8583_build(&message, &buffer);

    assert_equal("ISO8583 - BUILD - build basic message", POS_SUCCESS, result);

    // MTI: 4 bytes 
    assert_equal("ISO8583 - BUILD - build MTI", 1, memcmp(buffer.data, "0200", 4) == 0);

    // Bitmap primário: 8 bytes
    assert_equal("ISO8583 - BUILD - build primary bitmap", 1, buffer.data[4] == 0x70);

    // Length PAN 
    assert_equal("ISO8583 - BUILD - build PAN", 1, memcmp(buffer.data + 12, "16", 2) == 0);

    // PAN 
    assert_equal("ISO8583 - BUILD - build PAN", 1, memcmp(buffer.data + 14, "4111111111111111", 16) == 0);

    // Processing Code
    assert_equal("ISO8583 - BUILD - build processing code", 1, memcmp(buffer.data + 30, "000000", ISO8583_SIZE_PROCESSING_CODE) == 0);

    // Amount
    assert_equal("ISO8583 - BUILD - build amount", 1, memcmp(buffer.data + 36, "000000015000", ISO8583_SIZE_AMOUNT) == 0);

    assert_equal("ISO8583 - BUILD - build message length", 48, (int)buffer.length);

}

void test_build_invalid_arguments(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    assert_equal("ISO8583 - BUILD - NULL message", POS_ERROR_NULL_POINTER, iso8583_build(NULL, &buffer));

    assert_equal("ISO8583 - BUILD - NULL buffer",POS_ERROR_NULL_POINTER, iso8583_build(&message, NULL));

    strcpy(message.mti, "0100");

    assert_equal("ISO8583 - BUILD - invalid MTI", POS_ERROR_INVALID_MTI, iso8583_build(&message, &buffer));

    strcpy(message.mti, "02000");

    assert_equal("ISO8583 - BUILD - MTI invalid length", POS_ERROR_INVALID_MTI, iso8583_build(&message, &buffer));
}


void test_parse_success(void) {
    unsigned char response_data[] = {
        '0', '2', '1', '0',

        0x30, 0x20, 0x00, 0x00,
        0x0A, 0x00, 0x00, 0x00,

        '0', '0', '0', '0', '0', '0',

        '0', '0', '0', '0', '0', '0', '0', '1', '5', '0', '0', '0',

        '1', '2', '3', '4', '5', '6',

        '1', '2', '3', '4', '5', '6',
        '7', '8', '9', '0', '1', '2',

        '0', '0'
    };

    ISO8583Message message = {0};

    int result = iso8583_parse(
        response_data,
        sizeof(response_data),
        &message
    );

     
    assert_equal("ISO8583 - PARSE - parse success", POS_SUCCESS, result);

    assert_equal("ISO8583 - PARSE - parse MTI", 1, strcmp(message.mti, "0210") == 0);

    assert_equal("ISO8583 - PARSE - parse processing code", 1, strcmp(message.processing_code, "000000") == 0);

    assert_equal("ISO8583 - PARSE - parse amount", 1, strcmp(message.amount, "000000015000") == 0);

    assert_equal("ISO8583 - PARSE - parse STAN", 1, strcmp(message.stan, "123456") == 0);

    assert_equal("ISO8583 - PARSE - parse retrieval reference", 1, strcmp(message.retrieval_reference, "123456789012") == 0);

    assert_equal("ISO8583 - PARSE - parse response code", 1, strcmp(message.response_code, "00") == 0);
}

void test_parse_success_with_pan(void) {

    unsigned char request_data[] = {
        '0', '2', '0', '0',

        // Bitmap primário: campos 2, 3 e 4
        0x70, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        // Campo 2 - PAN (LLVAR)
        '1', '6',
        '4', '1', '1', '1', '1', '1', '1',
        '1', '1', '1', '1', '1', '1', '1', '1', '1',

        // Campo 3 - Processing Code
        '0', '0', '0', '0', '0', '0',

        // Campo 4 - Amount
        '0', '0', '0', '0', '0', '0',
        '0', '1', '5', '0', '0', '0'
    };

    ISO8583Message message = {0};

    int result = iso8583_parse(
        request_data,
        sizeof(request_data),
        &message
    );

    assert_equal("ISO8583 - PARSE - parse PAN success", POS_SUCCESS, result);

    assert_equal("ISO8583 - PARSE - parse PAN value", 1, strcmp(message.pan, "4111111111111111") == 0);

    assert_equal("ISO8583 - PARSE - parse processing code with PAN", 1, strcmp(message.processing_code, "000000") == 0);

    assert_equal("ISO8583 - PARSE - parse amount with PAN", 1, strcmp(message.amount, "000000015000") == 0);
}

void test_parse_invalid_pan(void) {
    unsigned char request_data[] = {
        '0', '2', '0', '0',
        // Bitmap primário: campos 2, 3 e 4
        0x70, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,

        // Campo 2 - PAN (LLVAR): 16 dígitos
        '1', '6',
        '4', '1', '1', '1', '1', '1', '1', '1',
        '1', '1', '1', '1', '1', '1', '1', '2',

        // Campo 3 - Processing Code
        '0', '0', '0', '0', '0', '0',

        // Campo 4 - Amount
        '0', '0', '0', '0', '0', '0',
        '0', '1', '5', '0', '0', '0'
    };

    ISO8583Message message = {0};

    int result = iso8583_parse(request_data, sizeof(request_data), &message);

    assert_equal("ISO8583 - PARSE - parse invalid PAN", POS_ERROR_INVALID_PAN, result);
}

void teste_parse_null_data(void) {
    
    ISO8583Message message = {0};
    
    int result = iso8583_parse(NULL, 12, &message);
    
    assert_equal("ISO8583 - PARSE - parse null data", POS_ERROR_NULL_POINTER, result);
}

void test_parse_null_message(void) {
    unsigned char data[12] = {0};

    int result = iso8583_parse(data, sizeof(data), NULL);

    assert_equal("ISO8583 - PARSE - parse null message", POS_ERROR_NULL_POINTER, result);
}

void test_build_secondary_bitmap(void) {
    ISO8583Message message = {0};
    ISO8583Buffer buffer = {0};

    strcpy(message.mti, "0200");
    strcpy(message.mac, "1234567890ABCDEF");

    int result = iso8583_build(&message, &buffer);

    assert_equal("ISO8583 - BUILD - secondary bitmap", POS_SUCCESS, result);

    assert_equal("ISO8583 - BUILD - secondary bitmap length", 36, buffer.length);

    assert_equal("ISO8583 - BUILD - secondary bitmap indicator", 0x80, buffer.primary_bitmap[0] & 0x80);

    assert_equal("ISO8583 - BUILD - field 128 bitmap", 0x01, buffer.secondary_bitmap[7]);

    assert_equal("ISO8583 - BUILD - primary bitmap in message", 0x80, buffer.data[4]);

    assert_equal("ISO8583 - BUILD - secondary bitmap in message", 0x01, buffer.data[19]);

    assert_equal("ISO8583 - BUILD - MAC field", 0, memcmp(&buffer.data[20], message.mac, ISO8583_SIZE_MAC));
}

void test_parse_secondary_bitmap(void) {
    ISO8583Message original = {0};
    ISO8583Message parsed = {0};
    ISO8583Buffer buffer = {0};

    strcpy(original.mti, "0200");
    strcpy(original.mac, "1234567890ABCDEF");

    POSStatus status = iso8583_build(&original, &buffer);

    assert_equal("ISO8583 - PARSE - prepare secondary bitmap", POS_SUCCESS, status);

    status = iso8583_parse(buffer.data, buffer.length, &parsed);

    assert_equal("ISO8583 - PARSE - secondary bitmap", POS_SUCCESS, status);

    assert_equal("ISO8583 - PARSE - MTI with secondary bitmap", 0, strcmp(original.mti, parsed.mti));

    assert_equal("ISO8583 - PARSE - MAC field", 0, strcmp(original.mac, parsed.mac));
}

void test_parse_incomplete_secondary_bitmap(void) {
    unsigned char data[12] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    data[4] = 0x80;

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - incomplete secondary bitmap", POS_ERROR_INVALID_FIELD_LENGTH, status);
}

void test_parse_unsupported_field(void) {
    unsigned char data[12] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    data[4] = 0x08;

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - unsupported field", POS_ERROR_INVALID_FIELD, status);
}

void test_parse_invalid_mti(void) {
    unsigned char data[12] = {0};
    ISO8583Message message = {0};

    memcpy(data, "9999", 4);

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - invalid MTI", POS_ERROR_INVALID_MTI, status);
}

void test_parse_short_message(void) {
    unsigned char data[11] = {0};
    ISO8583Message message = {0};

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - message shorter than minimum", POS_ERROR_INVALID_FIELD_LENGTH, status);
}

void test_parse_incomplete_primary_bitmap(void) {
    unsigned char data[11] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - incomplete primary bitmap", POS_ERROR_INVALID_FIELD_LENGTH, status);
}

void test_parse_incomplete_pan_length(void) {
    unsigned char data[12] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    data[4] = 0x40;

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - incomplete PAN length", POS_ERROR_INVALID_FIELD_LENGTH, status);
}

void test_parse_pan_zero_length(void) {
    unsigned char data[14] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    data[4] = 0x40;

    data[12] = '0';
    data[13] = '0';

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - PAN zero length", POS_ERROR_INVALID_FIELD_LENGTH, status);
}

void test_parse_extra_bytes(void) {
    unsigned char data[13] = {0};
    ISO8583Message message = {0};

    memcpy(data, "0200", 4);

    // Mensagem sem campos: MTI + bitmap primário.
    // O último byte é um dado excedente.
    data[12] = 0xFF;

    POSStatus status = iso8583_parse(data, sizeof(data), &message);

    assert_equal("ISO8583 - PARSE - extra bytes", POS_ERROR_INVALID_ARGUMENT, status);
}