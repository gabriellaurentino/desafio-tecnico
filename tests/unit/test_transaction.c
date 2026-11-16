#include "test_utils.h"
#include "transaction.h"
#include <stdio.h>

void test_transaction_init(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    assert_equal("TRANSACTION - INIT ", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - INIT REVERSAL", 0, transaction.reversal_required);

}

void test_transaction_init_null(void) {
    transaction_init(NULL);

    assert_equal("TRANSACTION - INIT null", 0, 0);
}

void test_transaction_start(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_START);

    assert_equal("TRANSACTION - START STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - START STATE", POS_STATE_CARD, transaction.state);
}

void test_transaction_card_read(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_CARD_READ);

    assert_equal("TRANSACTION - CARD READ STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - CARD READ STATE", POS_STATE_ISO_SEND, transaction.state);
}

void test_transaction_request_sent(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    assert_equal("TRANSACTION - REQUEST SENT STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - REQUEST SENT STATE", POS_STATE_WAIT_RESP, transaction.state);
}

void test_transaction_response_approved(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    assert_equal("TRANSACTION - APPROVED STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - APPROVED STATE", POS_STATE_APPROVED, transaction.state);
}

void test_transaction_response_declined(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESPONSE_DECLINED);

    assert_equal("TRANSACTION - DECLINED STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - DECLINED STATE", POS_STATE_DECLINED, transaction.state);
}

void test_transaction_timeout(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    assert_equal("TRANSACTION - TIMEOUT STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - TIMEOUT STATE", POS_STATE_REVERSAL, transaction.state);
    assert_equal("TRANSACTION - REVERSAL REQUIRED", 1, transaction.reversal_required);
}

void test_transaction_reset_with_pending_reversal(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET PENDING REVERSAL", POS_ERROR_INVALID_ARGUMENT, status);
    assert_equal("TRANSACTION - RESET STATE", POS_STATE_REVERSAL, transaction.state);
    assert_equal("TRANSACTION - RESET REVERSAL", 1, transaction.reversal_required);
}

void test_transaction_reversal_completed(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_REVERSAL_COMPLETED);

    assert_equal("TRANSACTION - REVERSAL COMPLETED STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - REVERSAL COMPLETED STATE", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - REVERSAL COMPLETED FLAG", 0, transaction.reversal_required);
}

void test_transaction_reset_approved(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET APPROVED STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - RESET APPROVED STATE", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - RESET APPROVED REVERSAL", 0, transaction.reversal_required);
}

void test_transaction_reset_declined(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_RESPONSE_DECLINED);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET DECLINED STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - RESET DECLINED STATE", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - RESET DECLINED REVERSAL", 0, transaction.reversal_required);
}

void test_transaction_reversal_completed_without_pending(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_REVERSAL_COMPLETED);

    assert_equal("TRANSACTION - REVERSAL WITHOUT PENDING STATUS", POS_ERROR_INVALID_ARGUMENT, status);
    assert_equal("TRANSACTION - REVERSAL WITHOUT PENDING STATE", POS_STATE_WAIT_RESP, transaction.state);
    assert_equal("TRANSACTION - REVERSAL WITHOUT PENDING FLAG", 0, transaction.reversal_required);
}

void test_transaction_invalid_event(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_CARD_READ);

    assert_equal("TRANSACTION - INVALID EVENT STATUS", POS_ERROR_INVALID_ARGUMENT, status);
    assert_equal("TRANSACTION - INVALID EVENT STATE", POS_STATE_IDLE, transaction.state);
}

void test_transaction_process_event_null(void) {

    POSStatus status = transaction_process_event(NULL, POS_EVENT_START);

    assert_equal("TRANSACTION - PROCESS EVENT NULL", POS_ERROR_NULL_POINTER, status);
}

void test_transaction_late_response_after_timeout(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_TIMEOUT);
    
    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    assert_equal("TRANSACTION - LATE RESPONSE STATUS", POS_ERROR_INVALID_ARGUMENT, status);
    assert_equal("TRANSACTION - LATE RESPONSE STATE", POS_STATE_REVERSAL, transaction.state);
    assert_equal("TRANSACTION - LATE RESPONSE REVERSAL", 1, transaction.reversal_required);

}

void test_transaction_reset_card(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET CARD STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - RESET CARD STATE", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - RESET CARD REVERSAL", 0, transaction.reversal_required);
}

void test_transaction_reset_iso_send(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET ISO SEND STATUS", POS_SUCCESS, status);
    assert_equal("TRANSACTION - RESET ISO SEND STATE", POS_STATE_IDLE, transaction.state);
    assert_equal("TRANSACTION - RESET ISO SEND REVERSAL", 0, transaction.reversal_required);
}

void test_transaction_reset_wait_response(void) {

    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESET);

    assert_equal("TRANSACTION - RESET WAIT RESP STATUS", POS_ERROR_INVALID_ARGUMENT, status);
    assert_equal("TRANSACTION - RESET WAIT RESP STATE", POS_STATE_WAIT_RESP, transaction.state);
    assert_equal("TRANSACTION - RESET WAIT RESP REVERSAL", 0, transaction.reversal_required);
}

void test_transaction_invalid_event_wait_response(void) {
    POSTransaction transaction;

    transaction_init(&transaction);
    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_START);

    assert_equal("TRANSACTION - INVALID EVENT WAIT RESP", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - INVALID EVENT WAIT RESP STATE", POS_STATE_WAIT_RESP, transaction.state);
}

void test_transaction_duplicate_timeout(void) {
    POSTransaction transaction;

    transaction_init(&transaction);
    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    assert_equal("TRANSACTION - DUPLICATE TIMEOUT", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - DUPLICATE TIMEOUT STATE", POS_STATE_REVERSAL, transaction.state);

    assert_equal("TRANSACTION - DUPLICATE TIMEOUT REVERSAL", 1, transaction.reversal_required);
}

void test_transaction_invalid_event_approved(void) {
    POSTransaction transaction;

    transaction_init(&transaction);
    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_START);

    assert_equal("TRANSACTION - INVALID EVENT APPROVED", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - INVALID EVENT APPROVED STATE", POS_STATE_APPROVED, transaction.state);
}

void test_transaction_invalid_state(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction.state = (POSTransactionState)99;

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_START);

    assert_equal("TRANSACTION - INVALID STATE", POS_ERROR_INVALID_FORMAT, status);
}

void test_transaction_invalid_event_iso_send(void) {
    POSTransaction transaction;

    transaction_init(&transaction);
    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_START);

    assert_equal("TRANSACTION - INVALID EVENT ISO SEND", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - INVALID EVENT ISO SEND STATE", POS_STATE_ISO_SEND, transaction.state);
}

void test_transaction_events_during_reversal(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction_process_event(&transaction, POS_EVENT_START);
    transaction_process_event(&transaction, POS_EVENT_CARD_READ);
    transaction_process_event(&transaction, POS_EVENT_REQUEST_SENT);
    transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    POSStatus approved = transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    POSStatus declined = transaction_process_event(&transaction, POS_EVENT_RESPONSE_DECLINED);

    POSStatus timeout = transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    assert_equal("TRANSACTION - REVERSAL BLOCK APPROVED", POS_ERROR_INVALID_ARGUMENT, approved);

    assert_equal("TRANSACTION - REVERSAL BLOCK DECLINED", POS_ERROR_INVALID_ARGUMENT, declined);

    assert_equal("TRANSACTION - REVERSAL BLOCK TIMEOUT", POS_ERROR_INVALID_ARGUMENT, timeout);

    assert_equal("TRANSACTION - REVERSAL STATE", POS_STATE_REVERSAL, transaction.state);

    assert_equal("TRANSACTION - REVERSAL FLAG", 1, transaction.reversal_required);
}

void test_transaction_approved_with_pending_reversal_in_wait_response(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction.state = POS_STATE_WAIT_RESP;
    transaction.reversal_required = 1;

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESPONSE_APPROVED);

    assert_equal("TRANSACTION - APPROVED WITH PENDING REVERSAL", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - APPROVED WITH PENDING REVERSAL STATE", POS_STATE_WAIT_RESP, transaction.state);

    assert_equal("TRANSACTION - APPROVED WITH PENDING REVERSAL FLAG", 1, transaction.reversal_required);
}

void test_transaction_declined_with_pending_reversal_in_wait_response(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction.state = POS_STATE_WAIT_RESP;
    transaction.reversal_required = 1;

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_RESPONSE_DECLINED);

    assert_equal("TRANSACTION - DECLINED WITH PENDING REVERSAL", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - DECLINED WITH PENDING REVERSAL STATE", POS_STATE_WAIT_RESP, transaction.state);

    assert_equal("TRANSACTION - DECLINED WITH PENDING REVERSAL FLAG", 1, transaction.reversal_required);
}

void test_transaction_timeout_with_pending_reversal_in_wait_response(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction.state = POS_STATE_WAIT_RESP;
    transaction.reversal_required = 1;

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_TIMEOUT);

    assert_equal("TRANSACTION - TIMEOUT WITH PENDING REVERSAL", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - TIMEOUT WITH PENDING REVERSAL STATE", POS_STATE_WAIT_RESP, transaction.state);

    assert_equal("TRANSACTION - TIMEOUT WITH PENDING REVERSAL FLAG", 1, transaction.reversal_required);
}

void test_transaction_reversal_completed_without_flag(void) {
    POSTransaction transaction;

    transaction_init(&transaction);

    transaction.state = POS_STATE_REVERSAL;
    transaction.reversal_required = 0;

    POSStatus status = transaction_process_event(&transaction, POS_EVENT_REVERSAL_COMPLETED);

    assert_equal("TRANSACTION - REVERSAL COMPLETED WITHOUT FLAG", POS_ERROR_INVALID_ARGUMENT, status);

    assert_equal("TRANSACTION - REVERSAL COMPLETED WITHOUT FLAG STATE", POS_STATE_REVERSAL, transaction.state);

    assert_equal("TRANSACTION - REVERSAL COMPLETED WITHOUT FLAG VALUE", 0, transaction.reversal_required);
}