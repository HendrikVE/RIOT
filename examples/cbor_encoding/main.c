/*
 * Copyright (C) 2022 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example for encoding and parsing CBOR data
 *
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "cbor.h"

#define ENABLE_DEBUG !BENCHMARK_BUILD
#include "debug.h"

#ifndef CONFIG_COMPILE_ENCODER
#define CONFIG_COMPILE_ENCODER          1
#endif

#ifndef CONFIG_COMPILE_FULL_PARSER
#define CONFIG_COMPILE_FULL_PARSER      1
#endif

#ifndef CONFIG_COMPILE_PARTIAL_PARSER
#define CONFIG_COMPILE_PARTIAL_PARSER   1
#endif

#define KEY_STUDENT_NAME    "student_name"
#define KEY_STUDENT_ID      "student_id"
#define KEY_GRADE           "grade"

#define MAX_KEY_LENGTH      sizeof(KEY_STUDENT_NAME)
#define ENCODER_BUF_LENGTH  64

typedef struct {
    char *student_name; /* mandatory */
    int  student_id;    /* mandatory */
    int  *grade;        /* optional */
} thesis_t;

#if CONFIG_COMPILE_ENCODER
static int _encode_thesis_t(thesis_t* thesis, uint8_t *encoder_buf, size_t encoder_buf_size)
{
    CborEncoder encoder;
    CborEncoder map_encoder;

    /* initialize encoder */
    cbor_encoder_init(&encoder, encoder_buf, encoder_buf_size, 0);

    /* total of 3 possible members thesis_t */
    int map_size = 3;

    /* grade is optional */
    if (thesis->grade == NULL) map_size--;

    /* start encoding root object */
    cbor_encoder_create_map(&encoder, &map_encoder, map_size);

    /* encode key-value pair */
    assert(thesis->student_name != NULL);
    cbor_encode_text_stringz(&map_encoder, KEY_STUDENT_NAME);
    cbor_encode_text_stringz(&map_encoder, thesis->student_name);

    /* encode key-value pair */
    cbor_encode_text_stringz(&map_encoder, KEY_STUDENT_ID);
    cbor_encode_int(&map_encoder, thesis->student_id);

    /* encode key-value pair if not NULL */
    if (thesis->grade != NULL) {
        cbor_encode_text_stringz(&map_encoder, KEY_GRADE);
        cbor_encode_int(&map_encoder, *(thesis->grade));
        /* Alternatively cbor_encode_null() may be used to explicitly encode a null value, but this
         * would mean that the key must be encoded as well which leads to more encoded bytes.  */
    }

    /* finish encoding root object */
    cbor_encoder_close_container(&encoder, &map_encoder);

    /* return the number of encoded bytes */
    return cbor_encoder_get_buffer_size(&encoder, encoder_buf);
}
#endif /* CONFIG_COMPILE_ENCODER */

#if CONFIG_COMPILE_FULL_PARSER
static int _parse_thesis_t(thesis_t* thesis, const uint8_t *parser_buf, size_t parser_buf_size)
{
    int err;

    CborParser parser;
    CborValue iterator;
    CborValue map_iterator;
    size_t map_size = 0;
    size_t key_len = 0;

    /* initialize parser */
    err = cbor_parser_init(parser_buf, parser_buf_size, 0, &parser, &iterator);
    if (err) return 1;

    /* check if root is map */
    if (!cbor_value_is_map(&iterator)) return 1;
    if (!cbor_value_is_length_known(&iterator)) return 1;
    err = cbor_value_get_map_length(&iterator, &map_size);
    if (err) return 1;

    err = cbor_value_enter_container(&iterator, &map_iterator);
    if (err) return 1;

    /* By using a loop, the parser is not dependent on the order of the items and is therefore
     * more flexible. */
    for (size_t i = 0; i < map_size; i++) {

        char key_buf[MAX_KEY_LENGTH];
        size_t key_buf_len = sizeof(key_buf);

        /* keys in cbor data have to be strings */
        if (!cbor_value_is_text_string(&map_iterator)) return 1;
        /* check for key length limit */
        if (!cbor_value_get_string_length(&map_iterator, &key_len)) {
            cbor_value_calculate_string_length(&map_iterator, &key_len);
            if (err) return 1;
        }
        /* key_buf[MAX_KEY_LENGTH] must be big enough for '\0' at the end */
        if (key_len > MAX_KEY_LENGTH - 1) return 1;
        /* get the string key */
        err = cbor_value_copy_text_string(&map_iterator, key_buf, &key_buf_len, NULL);
        if (err) return 1;

        /* advance from the key to the value */
        err = cbor_value_advance(&map_iterator);
        if (err) return 1;

        if (strcmp(key_buf, KEY_STUDENT_NAME) == 0) {
            size_t len;
            /* cbor_value_dup_text_string uses malloc to allocate the string, but providing a
             * statically allocated buffer for every single member of the struct may be infeasible.
             * NOTE: this function is not supported in RIOT by default */
            cbor_value_dup_text_string(&map_iterator, &(thesis->student_name), &len, NULL);
        }
        else if (strcmp(key_buf, KEY_STUDENT_ID) == 0) {
            /* check that value is of type integer */
            if (!cbor_value_is_integer(&map_iterator)) return 1;
            err = cbor_value_get_int(&map_iterator, &thesis->student_id);
            if (err) return 1;
        }
        else if (strcmp(key_buf, KEY_GRADE) == 0) {
            /* check that value is of type integer */
            if (cbor_value_is_integer(&map_iterator)) {
                err = cbor_value_get_int(&map_iterator, thesis->grade);
                if (err) return 1;
            }
            else if (cbor_value_is_null(&map_iterator)) {
                /* grade is allowed to be null */
                thesis->grade = NULL;
            }
            else {
                return 1;
            }
        }
        else {
            return 1;
        }

        if (i != map_size - 1) {
            /* advance to the next key */
            err = cbor_value_advance(&map_iterator);
            if (err) return 1;
        }
    }

    return 0;
}
#endif /* CONFIG_COMPILE_FULL_PARSER */

#if CONFIG_COMPILE_PARTIAL_PARSER
static int _parse_student_id_from_thesis_t(int *student_id,
                                           const uint8_t *parser_buf, size_t parser_buf_size)
{
    int err;

    CborParser parser;
    CborValue iterator;
    CborValue map_iterator;

    /* initialize parser */
    err = cbor_parser_init(parser_buf, parser_buf_size, 0, &parser, &iterator);
    if (err) return 1;

    /* check if root is map */
    if (!cbor_value_is_map(&iterator)) return 1;

    err = cbor_value_enter_container(&iterator, &map_iterator);
    if (err) return 1;

    /* NOTE: Here it is assumed that student_id is always the second key, so */
    /* advance three times to the value of student_id */
    err = cbor_value_advance(&map_iterator);
    if (err) return 1;
    err = cbor_value_advance(&map_iterator);
    if (err) return 1;
    err = cbor_value_advance(&map_iterator);
    if (err) return 1;

    /* check that value is of type integer */
    if (!cbor_value_is_integer(&map_iterator)) return 1;
    err = cbor_value_get_int(&map_iterator, student_id);
    if (err) return 1;

    return 0;
}
#endif /* CONFIG_COMPILE_PARTIAL_PARSER */

int main(void)
{
    int byte_count;
    (void)byte_count;

#if CONFIG_COMPILE_ENCODER
    thesis_t thesis = {
        .student_id = 4884349,
        .student_name = "Hendrik van Essen",

        /* grade is not known yet */
        .grade = NULL,
    };

    uint8_t encoder_buf[ENCODER_BUF_LENGTH];
    byte_count = _encode_thesis_t(&thesis, encoder_buf, sizeof(encoder_buf));
    DEBUG("Encoded %d bytes: [", byte_count);
    for (int i = 0; i < byte_count; i++) {
        DEBUG("%02x", encoder_buf[i]);
    }
    DEBUG("]\n");
#endif /* CONFIG_COMPILE_ENCODER */

#if CONFIG_COMPILE_FULL_PARSER || CONFIG_COMPILE_PARTIAL_PARSER
    int rc;
    (void)rc;

#if !CONFIG_COMPILE_ENCODER
    /* provide replacement data manually */
    uint8_t encoder_buf[ENCODER_BUF_LENGTH] = {
        0xa2, 0x6c, 0x73, 0x74, 0x75, 0x64, 0x65, 0x6e, 0x74, 0x5f, 0x6e, 0x61,
        0x6d, 0x65, 0x71, 0x48, 0x65, 0x6e, 0x64, 0x72, 0x69, 0x6b, 0x20, 0x76,
        0x61, 0x6e, 0x20, 0x45, 0x73, 0x73, 0x65, 0x6e, 0x6a, 0x73, 0x74, 0x75,
        0x64, 0x65, 0x6e, 0x74, 0x5f, 0x69, 0x64, 0x1a, 0x00, 0x4a, 0x87, 0x7d,
    };
    byte_count = 48;
#endif /* !CONFIG_COMPILE_ENCODER */

#endif /* CONFIG_COMPILE_FULL_PARSER || CONFIG_COMPILE_PARTIAL_PARSER */

#if CONFIG_COMPILE_FULL_PARSER
    thesis_t parsed_thesis = { 0 };
    rc = _parse_thesis_t(&parsed_thesis, encoder_buf, byte_count);
    assert(rc == 0);
    DEBUG("Parsed thesis: (%d, %s, %d)\n",
          parsed_thesis.student_id, parsed_thesis.student_name, parsed_thesis.grade == NULL);
#endif /* CONFIG_COMPILE_FULL_PARSER */

#if CONFIG_COMPILE_PARTIAL_PARSER
    int student_id;
    rc = _parse_student_id_from_thesis_t(&student_id, encoder_buf, byte_count);
    assert(rc == 0);
    DEBUG("Parsed student_id: %d\n", student_id);
#endif /* CONFIG_COMPILE_PARTIAL_PARSER */

    return 0;
}
