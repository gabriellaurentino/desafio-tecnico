#ifndef TEST_H
#define TEST_H


/* Luhn */
void test_valid_card_number(void);
void test_minimum_valid_card_number(void);
void test_maximum_valid_card_number(void);
void test_invalid_card_number(void);
void test_null_card_number(void);
void test_empty_card_number(void);
void test_bad_format_card_number(void);
void test_card_number_too_short(void);
void test_card_number_too_long(void);
void test_space_in_card_number(void);


/* ISO8583 */
void test_set_bitmap_field_2(void);
void test_set_bitmap_field_3(void);
void test_set_bitmap_field_64(void);
void test_set_bitmap_field_65(void);
void test_set_bitmap_field_128(void);
void test_set_bitmap_invalid_field_1(void);
void test_set_bitmap_invalid_field_129(void);
void test_set_bitmap_null_buffer(void);
void test_set_bitmap_same_field_twice(void);
void test_set_bitmap_preserves_other_bits(void);
void test_build_bitmap_all_fields(void);
void test_build_bitmap_only_pan(void);
void test_build_bitmap_only_mac(void);
void test_build_bitmap_empty_message(void);
void test_build_bitmap_invalid_pan(void);
void test_build_bitmap_fixed_field_too_short(void);
void test_build_bitmap_pan_variable_size(void);
void test_build_bitmap_null_message(void);
void test_build_bitmap_null_buffer(void);
void test_build_bitmap_preserves_original_on_error(void);
void test_build_bitmap_pan_too_short(void);
void test_build_bitmap_pan_too_long(void);
void test_build_bitmap_pan_with_letters(void);
void test_build_bitmap_processing_code_too_long(void);
void test_build_bitmap_amount_too_short(void);
void test_build_bitmap_amount_too_long(void);
void test_build_basic_message(void);
void test_build_invalid_arguments(void);
void test_build_secondary_bitmap(void);
void test_parse_success(void);
void test_parse_success_with_pan(void);
void test_parse_invalid_pan(void);
void teste_parse_null_data(void);
void test_parse_null_message(void);
void test_parse_secondary_bitmap(void);
void test_parse_incomplete_secondary_bitmap(void);
void test_parse_unsupported_field(void);
void test_parse_invalid_mti(void);
void test_parse_short_message(void);
void test_parse_incomplete_primary_bitmap(void);
void test_parse_incomplete_pan_length(void);
void test_parse_pan_zero_length(void);
void test_parse_extra_bytes(void);


/* String utils*/
void test_bounded_strlen_null(void);
void test_bounded_strlen_empty(void);
void test_bounded_strlen_normal(void);
void test_bounded_strlen_max_size(void);
void test_bounded_strlen_zero_max_size(void);

/* Transaction */
void test_transaction_init(void);
void test_transaction_init_null(void); 
void test_transaction_start(void);
void test_transaction_card_read(void);
void test_transaction_request_sent(void);
void test_transaction_response_approved(void);
void test_transaction_response_declined(void);
void test_transaction_timeout(void);
void test_transaction_reset_with_pending_reversal(void);
void test_transaction_reversal_completed(void);
void test_transaction_reset_approved(void);
void test_transaction_reset_declined(void);
void test_transaction_reversal_completed_without_pending(void);
void test_transaction_invalid_event(void);
void test_transaction_process_event_null(void);
void test_transaction_late_response_after_timeout(void);
void test_transaction_reset_card(void);
void test_transaction_reset_iso_send(void);
void test_transaction_reset_wait_response(void);
void test_transaction_invalid_event_wait_response(void);
void test_transaction_duplicate_timeout(void);
void test_transaction_invalid_event_approved(void);
void test_transaction_invalid_state(void);
void test_transaction_invalid_event_iso_send(void);
void test_transaction_events_during_reversal(void);
void test_transaction_approved_with_pending_reversal_in_wait_response(void);
void test_transaction_declined_with_pending_reversal_in_wait_response(void);
void test_transaction_timeout_with_pending_reversal_in_wait_response(void);
void test_transaction_reversal_completed_without_flag(void);


/* Flash Storage */
void test_flash_storage_init(void);
void test_flash_storage_init_memory(void);
void test_flash_storage_init_null(void);
void test_flash_storage_read_empty(void);
void test_flash_storage_read_address(void);
void test_flash_storage_read_null(void);
void test_flash_storage_read_invalid_size(void);
void test_flash_storage_read_invalid_address(void);
void test_flash_storage_read_out_of_bounds(void);
void test_flash_storage_write(void);
void test_flash_storage_write_multiple(void);
void test_flash_storage_write_invalid_bits(void);
void test_flash_storage_write_partial_failure(void);
void test_flash_storage_write_null(void);
void test_flash_storage_write_invalid_size(void);
void test_flash_storage_write_invalid_address(void);
void test_flash_storage_write_out_of_bounds(void);
void test_flash_storage_erase(void);
void test_flash_storage_erase_multiple(void);
void test_flash_storage_erase_preserve_data(void);
void test_flash_storage_erase_null(void);
void test_flash_storage_erase_invalid_size(void);
void test_flash_storage_erase_invalid_address(void);
void test_flash_storage_erase_out_of_bounds(void);
void test_flash_storage_write_after_erase(void);


/* Log */
void test_log_init_success(void);
void test_log_init_null_log(void);
void test_log_init_null_storage(void);
void test_log_write_without_pan(void);
void test_log_write_with_pan(void);
void test_log_write_pan_not_exposed(void);
void test_log_write_xor_encryption(void);
void test_log_write_null_log(void);
void test_log_write_null_storage(void);
void test_log_write_invalid_pan_length(void);
void test_log_write_invalid_pan_format(void);
void test_log_write_storage_full(void);
void test_log_read_success(void);
void test_log_read_multiple_records(void);
void test_log_reset_read_success(void);
void test_log_reset_read_null_log(void);
void test_log_reset_read_null_storage(void);
void test_log_write_invalid_response_code(void);
void test_log_write_invalid_status(void);
void test_log_write_response_code_invalid_length(void);
void test_log_read_invalid_record_size(void);
void test_log_read_incomplete_record(void);
void test_log_read_invalid_status(void);
void test_log_read_invalid_masked_pan(void);
void test_log_read_invalid_response_code(void);
void test_log_read_invalid_address(void);
void test_log_read_incomplete_header(void);
void test_log_read_write_address_out_of_bounds(void);


/* POS App */
void test_pos_app_init_success(void);
void test_pos_app_init_null_app(void);
void test_pos_app_init_null_storage(void);
void test_pos_app_start_transaction_success(void);
void test_pos_app_start_transaction_null(void);
void test_pos_app_start_transaction_already_started(void);
void test_pos_app_card_read_success(void);
void test_pos_app_card_read_invalid_pan(void);
void test_pos_app_card_read_null_app(void);
void test_pos_app_card_read_null_pan(void);
void test_pos_app_card_read_empty_pan(void);
void test_pos_app_card_read_pan_too_long(void);
void test_pos_app_card_read_invalid_state(void);
void test_pos_app_build_request_clock_failure(void);
void test_pos_app_set_clock_success(void);
void test_pos_app_set_clock_null_app(void);
void test_pos_app_set_clock_null_callback(void);
void test_pos_app_build_request_stan_first(void);
void test_pos_app_build_request_stan_next(void);
void test_pos_app_build_request_stan_rollover(void);
void test_pos_app_build_request_stan_last_value(void);
void test_pos_app_build_request_stan_without_clock(void);
void test_pos_app_build_request_stan_invalid_datetime(void);
void test_pos_app_build_request_stan_without_pan(void);
void test_pos_app_build_request_stan_without_pan(void);
void test_pos_app_build_request_stan_iso_build_failure(void);
void test_pos_app_reset_transaction_null_pointer(void);
void test_pos_app_reset_transaction_invalid_state(void);
void test_pos_app_reset_transaction_approved(void);
void test_pos_app_reset_transaction_declined(void);
void test_pos_app_reset_transaction_waiting_response(void);
void test_pos_app_reset_transaction_card(void);
void test_pos_app_reset_transaction_iso_send(void);


/* Integração */
void test_pos_app_transaction_build_request(void);
void test_pos_app_transaction_response_approved(void);
void test_pos_app_transaction_response_declined(void);
void test_pos_app_transaction_response_invalid_mti(void);
void test_pos_app_transaction_response_invalid_stan(void);
void test_pos_app_transaction_response_missing_code(void);
void test_pos_app_transaction_response_invalid_state(void);
void test_pos_app_transaction_response_malformed(void);
void test_pos_app_transaction_response_truncated(void);
void test_pos_app_transaction_response_invalid_code(void);
void test_pos_app_transaction_response_log_failure(void);
void test_pos_app_transaction_response_duplicate(void);
void test_pos_app_process_timeout(void);
void test_pos_app_process_timeout_duplicate(void);
void test_pos_app_process_timeout_invalid_state(void);
void test_pos_app_process_reversal_completed(void);
void test_pos_app_process_reversal_without_timeout(void);
void test_pos_app_process_timeout_log_failure(void);
void test_pos_app_process_reversal_log_failure(void);
void test_pos_app_process_late_response_after_timeout(void);
void test_pos_app_transaction_sequential(void);

#endif