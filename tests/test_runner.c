#include <stdio.h>
#include "test.h"
#include "test_utils.h"


int main(void) {
    printf("=============== POS LIBRARY TEST ===============\n");
    
    
    printf("\n------- Algoritmo Luhn -------\n");
    test_valid_card_number();
    test_minimum_valid_card_number();
    test_maximum_valid_card_number();
    test_invalid_card_number();
    test_null_card_number();
    test_bad_format_card_number();
    test_empty_card_number();
    test_card_number_too_short();
    test_card_number_too_long();
    test_space_in_card_number();

    
    printf("\n------- Mensageria ISO8583 -------\n");
    test_set_bitmap_field_2();
    test_set_bitmap_field_3();
    test_set_bitmap_field_64();
    test_set_bitmap_field_65();
    test_set_bitmap_field_128();
    test_set_bitmap_invalid_field_1();
    test_set_bitmap_invalid_field_129();
    test_set_bitmap_null_buffer();
    test_set_bitmap_same_field_twice();
    test_set_bitmap_preserves_other_bits();
    test_build_bitmap_all_fields();
    test_build_bitmap_only_pan();
    test_build_bitmap_only_mac();
    test_build_bitmap_empty_message();
    test_build_bitmap_invalid_pan();
    test_build_bitmap_fixed_field_too_short();
    test_build_bitmap_pan_variable_size();
    test_build_bitmap_null_message();
    test_build_bitmap_null_buffer();
    test_build_bitmap_preserves_original_on_error();
    test_build_bitmap_pan_too_short();
    test_build_bitmap_pan_too_long();
    test_build_bitmap_pan_with_letters();
    test_build_bitmap_processing_code_too_long();
    test_build_bitmap_amount_too_short();
    test_build_bitmap_amount_too_long();
    test_build_basic_message();
    test_build_invalid_arguments();
    test_build_secondary_bitmap();
    test_parse_success();
    test_parse_success_with_pan();
    test_parse_invalid_pan();
    teste_parse_null_data();
    test_parse_null_message();
    test_parse_secondary_bitmap();
    test_parse_incomplete_secondary_bitmap();
    test_parse_unsupported_field();
    test_parse_invalid_mti();
    test_parse_short_message();
    test_parse_incomplete_primary_bitmap();
    test_parse_incomplete_pan_length();
    test_parse_pan_zero_length();
    test_parse_extra_bytes();

    
    printf("\n------- STRING UTILS -------\n");
    test_bounded_strlen_null();
    test_bounded_strlen_empty();
    test_bounded_strlen_normal();
    test_bounded_strlen_max_size();
    test_bounded_strlen_zero_max_size();


    printf("\n------- TRANSACTION -------\n");
    test_transaction_init();
    test_transaction_init_null();
    test_transaction_start();
    test_transaction_card_read();
    test_transaction_request_sent();
    test_transaction_response_approved();
    test_transaction_response_declined();
    test_transaction_timeout();
    test_transaction_reset_with_pending_reversal();
    test_transaction_reversal_completed();
    test_transaction_reset_approved();
    test_transaction_reset_declined();
    test_transaction_reversal_completed_without_pending();
    test_transaction_invalid_event();
    test_transaction_process_event_null();
    test_transaction_late_response_after_timeout();
    test_transaction_reset_card();
    test_transaction_reset_iso_send();
    test_transaction_reset_wait_response();
    test_transaction_invalid_event_wait_response();
    test_transaction_duplicate_timeout();
    test_transaction_invalid_event_approved();
    test_transaction_invalid_state();
    test_transaction_invalid_event_iso_send();
    test_transaction_events_during_reversal();
    test_transaction_approved_with_pending_reversal_in_wait_response();
    test_transaction_declined_with_pending_reversal_in_wait_response();
    test_transaction_timeout_with_pending_reversal_in_wait_response();
    test_transaction_reversal_completed_without_flag();

    
    printf("\n------- FLASH STORAGE -------\n");
    test_flash_storage_init();
    test_flash_storage_init_memory();
    test_flash_storage_init_null();
    test_flash_storage_read_empty();
    test_flash_storage_read_address();
    test_flash_storage_read_null();
    test_flash_storage_read_invalid_size();
    test_flash_storage_read_invalid_address();
    test_flash_storage_read_out_of_bounds();
    test_flash_storage_write();
    test_flash_storage_write_multiple();
    test_flash_storage_write_invalid_bits();
    test_flash_storage_write_partial_failure();
    test_flash_storage_write_null();
    test_flash_storage_write_invalid_size();
    test_flash_storage_write_invalid_address();
    test_flash_storage_write_out_of_bounds();
    test_flash_storage_erase();
    test_flash_storage_erase_multiple();
    test_flash_storage_erase_preserve_data();
    test_flash_storage_erase_null();
    test_flash_storage_erase_invalid_size();
    test_flash_storage_erase_invalid_address();
    test_flash_storage_erase_out_of_bounds();
    test_flash_storage_write_after_erase();

    
    printf("\n------- LOG -------\n");
    test_log_init_success();
    test_log_init_null_log();
    test_log_init_null_storage();
    test_log_write_without_pan();
    test_log_write_with_pan();
    test_log_write_pan_not_exposed();
    test_log_write_xor_encryption();
    test_log_write_null_log();
    test_log_write_null_storage();
    test_log_write_invalid_pan_length();
    test_log_write_invalid_pan_format();
    test_log_write_storage_full();
    test_log_read_success();
    test_log_read_multiple_records();
    test_log_reset_read_success();
    test_log_reset_read_null_log();
    test_log_reset_read_null_storage();
    test_log_write_invalid_response_code();
    test_log_write_invalid_status();
    test_log_write_response_code_invalid_length();
    test_log_read_invalid_record_size();
    test_log_read_incomplete_record();
    test_log_read_invalid_status();
    test_log_read_invalid_masked_pan();
    test_log_read_invalid_response_code();
    test_log_read_invalid_address();
    test_log_read_incomplete_header();
    test_log_read_write_address_out_of_bounds();


    printf("\n------- POS APP -------\n");
    test_pos_app_init_success();
    test_pos_app_init_null_app();
    test_pos_app_init_null_storage();
    test_pos_app_start_transaction_success();
    test_pos_app_start_transaction_null();
    test_pos_app_start_transaction_already_started();
    test_pos_app_card_read_success();
    test_pos_app_card_read_invalid_pan();
    test_pos_app_card_read_null_app();
    test_pos_app_card_read_null_pan();
    test_pos_app_card_read_empty_pan();
    test_pos_app_card_read_pan_too_long();
    test_pos_app_card_read_invalid_state();
    test_pos_app_build_request_clock_failure();
    test_pos_app_set_clock_success();
    test_pos_app_set_clock_null_app();
    test_pos_app_set_clock_null_callback();
    test_pos_app_build_request_stan_first();
    test_pos_app_build_request_stan_next();
    test_pos_app_build_request_stan_rollover();
    test_pos_app_build_request_stan_last_value();
    test_pos_app_build_request_stan_without_clock();
    test_pos_app_build_request_stan_invalid_datetime();
    test_pos_app_build_request_stan_without_pan();
    test_pos_app_build_request_stan_iso_build_failure();
    test_pos_app_reset_transaction_null_pointer();
    test_pos_app_reset_transaction_invalid_state();
    test_pos_app_reset_transaction_approved();
    test_pos_app_reset_transaction_declined();
    test_pos_app_reset_transaction_waiting_response();
    test_pos_app_reset_transaction_card();
    test_pos_app_reset_transaction_iso_send();


    printf("\n------- INTEGRATION -------\n");
    test_pos_app_transaction_build_request();
    test_pos_app_transaction_response_approved();
    test_pos_app_transaction_response_declined();
    test_pos_app_transaction_response_invalid_mti();
    test_pos_app_transaction_response_invalid_stan();
    test_pos_app_transaction_response_missing_code();
    test_pos_app_transaction_response_invalid_state();
    test_pos_app_transaction_response_malformed();
    test_pos_app_transaction_response_truncated();
    test_pos_app_transaction_response_invalid_code();
    test_pos_app_transaction_response_log_failure();
    test_pos_app_transaction_response_duplicate();
    test_pos_app_process_timeout();
    test_pos_app_process_timeout_duplicate();
    test_pos_app_process_timeout_invalid_state();
    test_pos_app_process_reversal_completed();
    test_pos_app_process_reversal_without_timeout();
    test_pos_app_process_timeout_log_failure();
    test_pos_app_process_reversal_log_failure();
    test_pos_app_process_late_response_after_timeout();
    test_pos_app_transaction_sequential();


    printf("\n------- END TESTS -------\n");

    print_test_results();
    
    return 0;
}

