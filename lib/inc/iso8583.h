/**
 * @file iso8583.h
 * @author Gabriel Laurentino
 * @brief Interface para construção e interpretação de mensagens ISO8583.
 * @version 1.0
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 */

#ifndef ISO8583_H
#define ISO8583_H

#include "errors.h"
#include <stddef.h>

/**
 * @brief Capacidade máxima do buffer de mensagens ISO8583, em bytes.
 */
#define ISO8583_BUFFER_SIZE 512


/**
 * @name Números dos campos ISO8583 suportados.
 * @{
 */
#define ISO8583_FIELD_PAN                  2
#define ISO8583_FIELD_PROCESSING_CODE      3
#define ISO8583_FIELD_AMOUNT                4
#define ISO8583_FIELD_TRANSMISSION_DATE     7
#define ISO8583_FIELD_STAN                 11
#define ISO8583_FIELD_LOCAL_TIME           12
#define ISO8583_FIELD_RETRIEVAL_REFERENCE  37
#define ISO8583_FIELD_RESPONSE_CODE        39
#define ISO8583_FIELD_TERMINAL_ID           41
#define ISO8583_FIELD_CURRENCY_CODE         49
#define ISO8583_FIELD_MAC                  128
/** @} */


/**
 * @name Tamanhos máximos dos campos ISO8583, em caracteres.
 *
 * Os tamanhos não incluem o terminador nulo das strings.
 * @{
 */
#define ISO8583_SIZE_PAN                  19
#define ISO8583_SIZE_PROCESSING_CODE       6
#define ISO8583_SIZE_AMOUNT               12
#define ISO8583_SIZE_TRANSMISSION_DATE    10
#define ISO8583_SIZE_STAN                  6
#define ISO8583_SIZE_LOCAL_TIME            6
#define ISO8583_SIZE_RETRIEVAL_REFERENCE  12
#define ISO8583_SIZE_RESPONSE_CODE         2
#define ISO8583_SIZE_TERMINAL_ID           8
#define ISO8583_SIZE_CURRENCY_CODE         3
#define ISO8583_SIZE_MAC                  16
/** @} */


/**
 * @brief Representação estruturada de uma mensagem ISO8583.
 *
 * Armazena o MTI e os valores dos campos suportados pela implementação.
 * Os campos são representados como strings terminadas em nulo.
 *
 * A implementação suporta os MTIs 0200 e 0210.
 * O campo PAN utiliza formato LLVAR, enquanto os demais campos
 * são tratados como campos de tamanho fixo.
 */
typedef struct {
    char mti[5]; /**< Message Type Indicator (MTI), com 4 caracteres. */

    char pan[ISO8583_SIZE_PAN + 1]; /**< Campo 2: número do cartão (PAN). */
    char processing_code[ISO8583_SIZE_PROCESSING_CODE + 1]; /**< Campo 3: código de processamento. */
    char amount[ISO8583_SIZE_AMOUNT + 1]; /**< Campo 4: valor da transação. */
    char transmission_date[ISO8583_SIZE_TRANSMISSION_DATE + 1]; /**< Campo 7: data e horário de transmissão. */
    char stan[ISO8583_SIZE_STAN + 1]; /**< Campo 11: System Trace Audit Number. */
    char local_time[ISO8583_SIZE_LOCAL_TIME + 1]; /**< Campo 12: horário local. */
    char retrieval_reference[ISO8583_SIZE_RETRIEVAL_REFERENCE + 1]; /**< Campo 37: referência de recuperação. */
    char response_code[ISO8583_SIZE_RESPONSE_CODE + 1]; /**< Campo 39: código de resposta. */
    char terminal_id[ISO8583_SIZE_TERMINAL_ID + 1]; /**< Campo 41: identificador do terminal. */
    char currency_code[ISO8583_SIZE_CURRENCY_CODE + 1]; /**< Campo 49: código da moeda. */
    char mac[ISO8583_SIZE_MAC + 1]; /**< Campo 128: Message Authentication Code. */
} ISO8583Message;


/**
 * @brief Buffer utilizado na construção de mensagens ISO8583.
 *
 * Armazena os bitmaps primário e secundário, os dados serializados
 * da mensagem e o comprimento efetivamente utilizado.
 *
 * O conteúdo de data segue a sequência:
 * MTI + bitmap primário + bitmap secundário (quando presente)
 * + campos da mensagem.
 */
typedef struct {
    unsigned char primary_bitmap[8]; /**< Bitmap primário de 64 bits. */
    unsigned char secondary_bitmap[8]; /**< Bitmap secundário de 64 bits. */
    unsigned char data[ISO8583_BUFFER_SIZE]; /**< Dados serializados da mensagem. */
    size_t length; /**< Comprimento da mensagem serializada, em bytes. */
} ISO8583Buffer;


/**
 * @brief Define um campo no bitmap ISO8583.
 *
 * Atualiza o bitmap primário ou secundário de acordo com
 * o número do campo informado.
 *
 * Para campos acima de 64, ativa também o indicador de
 * presença do bitmap secundário no bitmap primário.
 *
 * @param[in,out] buffer Buffer cujos bitmaps serão atualizados.
 * @param[in] field Número do campo ISO8583.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se buffer for NULL.
 * @return POS_ERROR_INVALID_LENGTH se o número do campo
 *         estiver fora do intervalo permitido (2 a 128).
 */
POSStatus iso8583_set_bitmap_bit(ISO8583Buffer *buffer, int field);


/**
 * @brief Constrói os bitmaps de uma mensagem ISO8583.
 *
 * Analisa os campos preenchidos na estrutura da mensagem,
 * valida seus comprimentos e ativa os respectivos bits
 * nos bitmaps primário e secundário.
 *
 * O PAN é validado utilizando o algoritmo de Luhn.
 *
 * Esta função atualiza os bitmaps do buffer, mas não
 * serializa a mensagem completa.
 *
 * @param[in] message Mensagem contendo os campos a serem representados.
 * @param[out] buffer Buffer que receberá os bitmaps.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se message ou buffer forem NULL.
 * @return POS_ERROR_INVALID_FIELD_LENGTH se algum campo possuir
 *         comprimento inválido.
 * @return Código de erro retornado pela validação do PAN.
 */
POSStatus iso8583_build_bitmap(
    const ISO8583Message *message,
    ISO8583Buffer *buffer
);


/**
 * @brief Constrói uma mensagem ISO8583 serializada.
 *
 * Valida o MTI e os campos preenchidos, constrói os bitmaps
 * e serializa os dados no buffer de saída.
 *
 * A mensagem é composta por:
 * - MTI com 4 caracteres.
 * - Bitmap primário com 8 bytes.
 * - Bitmap secundário com 8 bytes, quando presente.
 * - Campos conforme a ordem definida pela implementação.
 *
 * O PAN é serializado no formato LLVAR.
 * Os demais campos suportados são serializados em formato fixo.
 *
 * Os MTIs aceitos são 0200 e 0210.
 *
 * @param[in] message Mensagem estruturada a ser serializada.
 * @param[out] buffer Buffer que receberá a mensagem construída.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se message ou buffer forem NULL.
 * @return POS_ERROR_INVALID_MTI se o MTI não for suportado.
 * @return POS_ERROR_INVALID_FIELD_LENGTH se algum campo possuir
 *         comprimento inválido.
 * @return POS_ERROR_INVALID_FIELD_FORMAT se o formato do campo
 *         não for suportado.
 * @return POS_ERROR_INVALID_LENGTH se a mensagem exceder
 *         a capacidade do buffer.
 * @return Código de erro retornado pela validação dos campos.
 */
POSStatus iso8583_build(
    const ISO8583Message *message,
    ISO8583Buffer *buffer
);


/**
 * @brief Interpreta uma mensagem ISO8583 serializada.
 *
 * Analisa os dados recebidos, identifica o MTI, interpreta
 * os bitmaps e extrai os campos suportados para a estrutura
 * ISO8583Message.
 *
 * Valida os MTIs, os campos presentes nos bitmaps, os
 * comprimentos dos campos e o PAN utilizando Luhn.
 *
 * A implementação aceita os MTIs 0200 e 0210 e rejeita
 * campos não suportados ou dados excedentes após a interpretação.
 *
 * A estrutura de destino somente é atualizada após
 * a interpretação completa da mensagem com sucesso.
 *
 * @param[in] data Dados serializados da mensagem ISO8583.
 * @param[in] length Comprimento dos dados recebidos, em bytes.
 * @param[out] message Estrutura que receberá os campos interpretados.
 *
 * @return POS_SUCCESS em caso de sucesso.
 * @return POS_ERROR_NULL_POINTER se data ou message forem NULL.
 * @return POS_ERROR_INVALID_MTI se o MTI não for suportado.
 * @return POS_ERROR_INVALID_BITMAP se aplicável à validação do bitmap.
 * @return POS_ERROR_INVALID_FIELD se houver campo não suportado.
 * @return POS_ERROR_INVALID_FIELD_LENGTH se algum campo estiver
 *         incompleto ou possuir comprimento inválido.
 * @return POS_ERROR_INVALID_FORMAT se o prefixo LLVAR possuir
 *         caracteres não numéricos.
 * @return POS_ERROR_INVALID_ARGUMENT se houver dados excedentes
 *         após a interpretação da mensagem.
 * @return Código de erro retornado pela validação do PAN.
 */
POSStatus iso8583_parse(
    const unsigned char *data,
    size_t length,
    ISO8583Message *message
);

#endif