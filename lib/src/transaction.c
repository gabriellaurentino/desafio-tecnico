/**
 * @file transaction.c
 * @author Gabriel Laurentino
 * @brief Implementação do gerenciamento de transações POS.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#include "transaction.h"

#include <stddef.h>


POSStatus transaction_init(POSTransaction *transaction) {
    
    if (transaction == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    transaction->state = POS_STATE_IDLE;
    transaction->reversal_required = 0;

    return POS_SUCCESS;
}

POSStatus transaction_process_event(POSTransaction *transaction, POSTransactionEvent event) {

    if (transaction == NULL) {
        return POS_ERROR_NULL_POINTER;
    }

    if (event == POS_EVENT_RESET && transaction->reversal_required) {
        return POS_ERROR_INVALID_ARGUMENT;
    }

    switch (transaction->state) {
        case POS_STATE_IDLE:
            
            if (event == POS_EVENT_START) {
                transaction->state = POS_STATE_CARD;
                return POS_SUCCESS;
            }
            
            break;
        
        case POS_STATE_CARD:
        
            if (event == POS_EVENT_CARD_READ) {
                transaction->state = POS_STATE_ISO_SEND;
                return POS_SUCCESS;
            }

            if (event == POS_EVENT_RESET) {
                transaction->state = POS_STATE_IDLE;
                transaction->reversal_required = 0;
                return POS_SUCCESS;
            }

            break;

        case POS_STATE_ISO_SEND:
        
            if (event == POS_EVENT_REQUEST_SENT) {
                transaction->state = POS_STATE_WAIT_RESP;
                return POS_SUCCESS;
            }

            if (event == POS_EVENT_RESET) {
                transaction->state = POS_STATE_IDLE;
                transaction->reversal_required = 0;
                return POS_SUCCESS;
            }

            break;

        case POS_STATE_WAIT_RESP:

            if (transaction->reversal_required && 
                (event == POS_EVENT_RESPONSE_APPROVED || event == POS_EVENT_RESPONSE_DECLINED)) {
                return POS_ERROR_INVALID_ARGUMENT;
            }

            if (event == POS_EVENT_RESPONSE_APPROVED) {
                transaction->state = POS_STATE_APPROVED;
                return POS_SUCCESS;
            }

            if (event == POS_EVENT_RESPONSE_DECLINED) {
                transaction->state = POS_STATE_DECLINED;
                return POS_SUCCESS;
            }

            if (event == POS_EVENT_TIMEOUT) {
                
                if (transaction->reversal_required) {
                    return POS_ERROR_INVALID_ARGUMENT;
                }

                transaction->reversal_required = 1;
                transaction->state = POS_STATE_REVERSAL;

                return POS_SUCCESS;
            }

            break;

        case POS_STATE_REVERSAL:

            if (event == POS_EVENT_REVERSAL_COMPLETED) {
                if (!transaction->reversal_required) {
                    return POS_ERROR_INVALID_ARGUMENT;
                }

                transaction->reversal_required = 0;
                transaction->state = POS_STATE_IDLE;

                return POS_SUCCESS;
            }

            break;

        case POS_STATE_APPROVED:
        case POS_STATE_DECLINED:
            
            if (event == POS_EVENT_RESET) {
                
                transaction->state = POS_STATE_IDLE;
                transaction->reversal_required = 0;

                return POS_SUCCESS;

            }

            break;

        default:
            return POS_ERROR_INVALID_FORMAT;
    }

    return POS_ERROR_INVALID_ARGUMENT;
}