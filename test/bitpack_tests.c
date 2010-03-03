#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CuTest.h"
#include "bitpack.h"

static void test_bitpack_constructor(CuTest *tc)
{
    bitpack_t  bp1 = NULL;
    bitpack_t  bp2 = NULL;
    char      *s1  = NULL;
    char      *s2  = NULL;
    unsigned char bytes[] = { 0xab, 0xcd, 0xef, 0x12 };

    bp1 = bitpack_init_default();
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp1, &s1));
    CuAssertPtrNotNull(tc, bp1);
    CuAssertPtrNotNull(tc, s1);
    CuAssertIntEquals(tc, 0, bitpack_size(bp1));
    CuAssertIntEquals(tc, BITPACK_DEFAULT_MEM_SIZE, bitpack_data_size(bp1));
    CuAssertStrEquals(tc, "", s1);

    bp2 = bitpack_init_from_bytes(bytes, 4);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp2, &s2));
    CuAssertPtrNotNull(tc, bp2);
    CuAssertPtrNotNull(tc, s2);
    CuAssertIntEquals(tc, 32, bitpack_size(bp2));
    CuAssertStrEquals(tc, "10101011110011011110111100010010", s2);

    bitpack_destroy(bp1);
    bitpack_destroy(bp2);
    free(s1);
    free(s2);
}

static void test_bitpack_get_on_off(CuTest *tc)
{
    bitpack_t  bp = NULL;
    char      *s  = NULL;
    int        i;
    unsigned char expected_bit;
    unsigned char test_bit;

    bp = bitpack_init_default();

    CuAssertIntEquals(tc, 0, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_on(bp, 0));
    CuAssertIntEquals(tc, 1, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 1));
    CuAssertIntEquals(tc, 2, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_on(bp, 2));
    CuAssertIntEquals(tc, 3, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 3));
    CuAssertIntEquals(tc, 4, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_on(bp, 4));
    CuAssertIntEquals(tc, 5, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 5));
    CuAssertIntEquals(tc, 6, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_on(bp, 6));
    CuAssertIntEquals(tc, 7, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 7));
    CuAssertIntEquals(tc, 8, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_on(bp, 8));
    CuAssertIntEquals(tc, 9, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 9));
    CuAssertIntEquals(tc, 10, bitpack_size(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));

    CuAssertStrEquals(tc, "1010101010", s);
    free(s);

    for (i = 0; i < 10; i++) {
        expected_bit = (i % 2 == 0) ? 1 : 0;
        CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get(bp, i, &test_bit));
        CuAssertIntEquals(tc, expected_bit, test_bit);
    }

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 0));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 2));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 4));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 6));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_off(bp, 8));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));

    CuAssertStrEquals(tc, "0000000000", s);
    free(s);

    for (i = 0; i < 10; i++) {
        CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get(bp, i, &test_bit));
        CuAssertIntEquals(tc, 0, test_bit);
    }

    /* error cases */
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get(bp, 10, &test_bit));
    CuAssertIntEquals(tc, BITPACK_ERR_INVALID_INDEX, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "invalid index (10), max index is 9", bitpack_get_error_str(bp));

    bitpack_destroy(bp);

    bp = bitpack_init_default();
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get(bp, 10, &test_bit));
    CuAssertIntEquals(tc, BITPACK_ERR_EMPTY, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "bitpack is empty", bitpack_get_error_str(bp));

    bitpack_destroy(bp);
}

static void test_bitpack_get_set_bits(CuTest *tc)
{
    bitpack_t  bp = NULL;
    char      *s  = NULL;
    char       err_str[BITPACK_ERR_BUF_SIZE];
    unsigned long value;

    bp = bitpack_init(4);

    CuAssertIntEquals(tc, 0, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0xff, 8, 0));
    CuAssertIntEquals(tc, 8, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "11111111", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bits(bp, 8, 0, &value));
    CuAssertIntEquals(tc, 0xff, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 5, 3, 8));
    CuAssertIntEquals(tc, 11, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "11111111101", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bits(bp, 3, 8, &value));
    CuAssertIntEquals(tc, 5, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 21, 5, 11));
    CuAssertIntEquals(tc, 16, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "1111111110110101", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bits(bp, 5, 11, &value));
    CuAssertIntEquals(tc, 21, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0xffffffff, 32, 16));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "111111111011010111111111111111111111111111111111", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bits(bp, 32, 16, &value));
    CuAssertIntEquals(tc, 0xffffffff, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0, 8, 0));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000001011010111111111111111111111111111111111", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bits(bp, 8, 0, &value));
    CuAssertIntEquals(tc, 0, value);

    /* error cases */
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_set_bits(bp, 0, sizeof(unsigned long) * 8 + 1, 0));
    CuAssertIntEquals(tc, BITPACK_ERR_RANGE_TOO_BIG, bitpack_get_error(bp));
    snprintf(err_str, sizeof(err_str), "range size %lu bits is too large (maximum size is %lu bits)",
            sizeof(unsigned long) * 8 + 1, sizeof(unsigned long) * 8);
    CuAssertStrEquals(tc, err_str, bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000001011010111111111111111111111111111111111", s);
    free(s);
    
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_set_bits(bp, 8, 3, 0));
    CuAssertIntEquals(tc, BITPACK_ERR_VALUE_TOO_BIG, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "value 8 does not fit in 3 bits", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000001011010111111111111111111111111111111111", s);
    free(s);

    value = 99;
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get_bits(bp, 3, 48, &value));
    CuAssertIntEquals(tc, BITPACK_ERR_INVALID_INDEX, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "invalid index (48), max index is 47", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 99, value);

    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get_bits(bp, 25, 24, &value));
    CuAssertIntEquals(tc, BITPACK_ERR_READ_PAST_END, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "attempted to read past end of bitpack (last index is 47)", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 99, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0xffffffff, 32, 48));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0xffffffff, 32, 80));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bits(bp, 0xffffffff, 32, 112));
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get_bits(bp, sizeof(unsigned long)*8+1, 0, &value));
    CuAssertIntEquals(tc, BITPACK_ERR_RANGE_TOO_BIG, bitpack_get_error(bp));
    snprintf(err_str, sizeof(err_str), "range size %lu bits is too large (maximum size is %lu bits)",
            sizeof(unsigned long) * 8 + 1, sizeof(unsigned long) * 8);
    CuAssertStrEquals(tc, err_str, bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 99, value);

    bitpack_destroy(bp);
}

static void test_bitpack_get_set_bytes(CuTest *tc)
{
    bitpack_t  bp = NULL;
    char      *s  = NULL;
    unsigned char test_bytes1[] = { 0x01, 0x02, 0x03 };
    unsigned char test_bytes2[] = { 0xff, 0xfe, 0xfd };
    unsigned char test_bytes3[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    unsigned char *bytes;

    bp = bitpack_init(4);

    CuAssertIntEquals(tc, 0, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bytes(bp, test_bytes1, 3, 0));
    CuAssertIntEquals(tc, 24, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000010000001000000011", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bytes(bp, 3, 0, &bytes));
    CuAssertTrue(tc, memcmp(test_bytes1, bytes, 3) == 0);
    free(bytes);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bytes(bp, test_bytes2, 3, 24));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000010000001000000011111111111111111011111101", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bytes(bp, 3, 24, &bytes));
    CuAssertTrue(tc, memcmp(test_bytes2, bytes, 3) == 0);
    free(bytes);

    /* non-byte aligned set */
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_set_bytes(bp, test_bytes3, 6, 50));
    CuAssertIntEquals(tc, 98, bitpack_size(bp));
    CuAssertIntEquals(tc, 13, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "00000001000000100000001111111111111111101111110100101010101011101111001100110111011110111011111111", s);
    free(s);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_get_bytes(bp, 6, 50, &bytes));
    CuAssertTrue(tc, memcmp(test_bytes3, bytes, 6) == 0);
    free(bytes);

    /* error cases */
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    bytes = NULL;
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get_bytes(bp, 5, 98, &bytes));
    CuAssertIntEquals(tc, BITPACK_ERR_INVALID_INDEX, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "invalid index (98), max index is 97", bitpack_get_error_str(bp));
    CuAssertPtrEquals(tc, bytes, NULL);

    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_get_bytes(bp, 13, 0, &bytes));
    CuAssertIntEquals(tc, BITPACK_ERR_READ_PAST_END, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "attempted to read past end of bitpack (last index is 97)", bitpack_get_error_str(bp));
    CuAssertPtrEquals(tc, bytes, NULL);

    bitpack_destroy(bp);
}

static void test_bitpack_append_bits(CuTest *tc)
{
    bitpack_t  bp = NULL;
    char      *s  = NULL;

    bp = bitpack_init(4);

    CuAssertIntEquals(tc, 0, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bits(bp, 0xff, 8));
    CuAssertIntEquals(tc, 8, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "11111111", s);
    free(s);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bits(bp, 5, 3));
    CuAssertIntEquals(tc, 11, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "11111111101", s);
    free(s);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bits(bp, 21, 5));
    CuAssertIntEquals(tc, 16, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "1111111110110101", s);
    free(s);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bits(bp, 0xffffffff, 32));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "111111111011010111111111111111111111111111111111", s);
    free(s);

    bitpack_destroy(bp);
}

static void test_bitpack_append_bytes(CuTest *tc)
{
    bitpack_t  bp = NULL;
    char      *s  = NULL;
    unsigned char test_bytes1[] = { 0x01, 0x02, 0x03 };
    unsigned char test_bytes2[] = { 0xff, 0xfe, 0xfd };
    unsigned char test_bytes3[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    bp = bitpack_init(4);

    CuAssertIntEquals(tc, 0, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bytes(bp, test_bytes1, 3));
    CuAssertIntEquals(tc, 24, bitpack_size(bp));
    CuAssertIntEquals(tc, 4, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000010000001000000011", s);
    free(s);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bytes(bp, test_bytes2, 3));
    CuAssertIntEquals(tc, 48, bitpack_size(bp));
    CuAssertIntEquals(tc, 6, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "000000010000001000000011111111111111111011111101", s);
    free(s);

    /* non-byte aligned set */
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bits(bp, 0, 2));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_append_bytes(bp, test_bytes3, 6));
    CuAssertIntEquals(tc, 98, bitpack_size(bp));
    CuAssertIntEquals(tc, 13, bitpack_data_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, "00000001000000100000001111111111111111101111110100101010101011101111001100110111011110111011111111", s);
    free(s);

    bitpack_destroy(bp);
}

static void test_bitpack_read_bits(CuTest *tc)
{
    bitpack_t  bp = NULL;
    unsigned long value;

    bp = bitpack_init(4);

    bitpack_append_bits(bp, 0xff, 8);
    bitpack_append_bits(bp, 5, 3);
    bitpack_append_bits(bp, 21, 5);
    bitpack_append_bits(bp, 0xffffffff, 32);

    CuAssertIntEquals(tc, 0, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 8, &value));
    CuAssertIntEquals(tc, 8, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 0xff, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 3, &value));
    CuAssertIntEquals(tc, 11, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 5, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 5, &value));
    CuAssertIntEquals(tc, 16, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 21, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 32, &value));
    CuAssertIntEquals(tc, 48, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 0xffffffff, value);

    /* error cases */
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    value = 0;
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_read_bits(bp, 1, &value));
    CuAssertIntEquals(tc, BITPACK_ERR_READ_PAST_END, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "attempted to read past end of bitpack (last index is 47)", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 48, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 0, value);

    bitpack_reset_read_pos(bp);
    CuAssertIntEquals(tc, 0, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 8, &value));
    CuAssertIntEquals(tc, 8, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 0xff, value);

    bitpack_destroy(bp);
}

static void test_bitpack_read_bytes(CuTest *tc)
{
    bitpack_t  bp = NULL;
    unsigned char test_bytes1[] = { 0x01, 0x02, 0x03 };
    unsigned char test_bytes2[] = { 0xff, 0xfe, 0xfd };
    unsigned char test_bytes3[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    unsigned char *bytes;
    unsigned long value;

    bp = bitpack_init(4);
    bitpack_append_bytes(bp, test_bytes1, 3);
    bitpack_append_bytes(bp, test_bytes2, 3);
    bitpack_append_bits(bp, 2, 2);
    bitpack_append_bytes(bp, test_bytes3, 6);

    CuAssertIntEquals(tc, 0, bitpack_read_pos(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, 3, &bytes));
    CuAssertIntEquals(tc, 24, bitpack_read_pos(bp));
    CuAssertTrue(tc, memcmp(test_bytes1, bytes, 3) == 0);
    free(bytes);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, 3, &bytes));
    CuAssertIntEquals(tc, 48, bitpack_read_pos(bp));
    CuAssertTrue(tc, memcmp(test_bytes2, bytes, 3) == 0);
    free(bytes);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 2, &value));
    CuAssertIntEquals(tc, 50, bitpack_read_pos(bp));
    CuAssertIntEquals(tc, 2, value);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, 6, &bytes));
    CuAssertIntEquals(tc, 98, bitpack_read_pos(bp));
    CuAssertTrue(tc, memcmp(test_bytes3, bytes, 6) == 0);
    free(bytes);

    /* error cases */
    CuAssertIntEquals(tc, BITPACK_ERR_CLEAR, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "", bitpack_get_error_str(bp));
    bytes = NULL;
    CuAssertIntEquals(tc, BITPACK_RV_ERROR, bitpack_read_bytes(bp, 1, &bytes));
    CuAssertIntEquals(tc, BITPACK_ERR_READ_PAST_END, bitpack_get_error(bp));
    CuAssertStrEquals(tc, "attempted to read past end of bitpack (last index is 97)", bitpack_get_error_str(bp));
    CuAssertIntEquals(tc, 98, bitpack_read_pos(bp));
    CuAssertPtrEquals(tc, NULL, bytes);

    bitpack_reset_read_pos(bp);
    CuAssertIntEquals(tc, 0, bitpack_read_pos(bp));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, 3, &bytes));
    CuAssertIntEquals(tc, 24, bitpack_read_pos(bp));
    CuAssertTrue(tc, memcmp(test_bytes1, bytes, 3) == 0);
    free(bytes);

    bitpack_destroy(bp);
}

static void test_bitpack_to_bytes(CuTest *tc)
{
    bitpack_t      bp = NULL;
    unsigned char *bytes = NULL;
    unsigned char  test_bytes1[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce };
    unsigned char  test_bytes2[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
    unsigned char  exp_bytes[] = { 
        0x1b, 0x91, 0xa2, 0xb3, 0xc0, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa,
        0xce, 0x03, 0x04, 0x11, 0x11, 0x11, 0x11, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    };
    unsigned char  exp_bytes2[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce, 0x80 };
    unsigned long num_bytes;
    char *s;
    char *exp_s = "0001101110010001101000101011001111000000110111101010110110111110111011111111111011101101111110101100111000000011000001000001000100010001000100010001000100000001000000100000001100000100000001010000011000000111000010000000100100001010";

    bp = bitpack_init_default();

    bitpack_append_bits(bp, 3, 5);
    bitpack_append_bits(bp, 3, 3);
    bitpack_append_bits(bp, 0x12345678, 29);
    bitpack_append_bits(bp, 0, 3);
    bitpack_append_bytes(bp, test_bytes1, sizeof(test_bytes1));
    bitpack_append_bits(bp, 12, 10);
    bitpack_append_bits(bp, 4, 6);
    bitpack_append_bits(bp, 0x11111111, 32);
    bitpack_append_bytes(bp, test_bytes2, sizeof(test_bytes2));

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bytes(bp, &bytes, &num_bytes));
    CuAssertPtrNotNull(tc, bytes);
    CuAssertIntEquals(tc, sizeof(exp_bytes) * 8, bitpack_size(bp));
    CuAssertIntEquals(tc, sizeof(exp_bytes), num_bytes);
    CuAssertTrue(tc, memcmp(exp_bytes, bytes, sizeof(exp_bytes)) == 0);
    free(bytes);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, exp_s, s);
    free(s);

    bitpack_destroy(bp);

    bp = bitpack_init(100);

    bitpack_append_bytes(bp, test_bytes1, sizeof(test_bytes1));
    bitpack_append_bits(bp, 1, 1);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bytes(bp, &bytes, &num_bytes));
    CuAssertPtrNotNull(tc, bytes);
    CuAssertIntEquals(tc, sizeof(test_bytes1) * 8 + 1, bitpack_size(bp));
    CuAssertIntEquals(tc, sizeof(test_bytes1) + 1, num_bytes);
    CuAssertTrue(tc, memcmp(exp_bytes2, bytes, sizeof(exp_bytes2)) == 0);
    free(bytes);

    bitpack_destroy(bp);
}

static void test_bitpack_from_bytes(CuTest *tc)
{
    bitpack_t      bp = NULL;
    unsigned char  test_bytes1[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce };
    unsigned char  test_bytes2[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a };
    unsigned char  init_bytes[] = { 
        0x1b, 0x91, 0xa2, 0xb3, 0xc0, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa,
        0xce, 0x03, 0x04, 0x11, 0x11, 0x11, 0x11, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    };
    char *s;
    char *exp_s = "0001101110010001101000101011001111000000110111101010110110111110111011111111111011101101111110101100111000000011000001000001000100010001000100010001000100000001000000100000001100000100000001010000011000000111000010000000100100001010";
    unsigned long value;
    unsigned char *bytes = NULL;

    bp = bitpack_init_from_bytes(init_bytes, sizeof(init_bytes));

    CuAssertIntEquals(tc, sizeof(init_bytes) * 8, bitpack_size(bp));
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_to_bin(bp, &s));
    CuAssertStrEquals(tc, exp_s, s);
    free(s);

    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 5, &value));
    CuAssertIntEquals(tc, 3, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 3, &value));
    CuAssertIntEquals(tc, 3, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 29, &value));
    CuAssertIntEquals(tc, 0x12345678, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 3, &value));
    CuAssertIntEquals(tc, 0, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, sizeof(test_bytes1), &bytes));
    CuAssertTrue(tc, memcmp(test_bytes1, bytes, sizeof(test_bytes1)) == 0);
    free(bytes);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 10, &value));
    CuAssertIntEquals(tc, 12, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 6, &value));
    CuAssertIntEquals(tc, 4, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bits(bp, 32, &value));
    CuAssertIntEquals(tc, 0x11111111, value);
    CuAssertIntEquals(tc, BITPACK_RV_SUCCESS, bitpack_read_bytes(bp, sizeof(test_bytes2), &bytes));
    CuAssertTrue(tc, memcmp(test_bytes2, bytes, sizeof(test_bytes2)) == 0);
    free(bytes);

    bitpack_destroy(bp);
}

static CuSuite *bitpack_get_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_bitpack_constructor);
    SUITE_ADD_TEST(suite, test_bitpack_get_on_off);
    SUITE_ADD_TEST(suite, test_bitpack_get_set_bits);
    SUITE_ADD_TEST(suite, test_bitpack_get_set_bytes);
    SUITE_ADD_TEST(suite, test_bitpack_append_bits);
    SUITE_ADD_TEST(suite, test_bitpack_append_bytes);
    SUITE_ADD_TEST(suite, test_bitpack_read_bits);
    SUITE_ADD_TEST(suite, test_bitpack_read_bytes);
    SUITE_ADD_TEST(suite, test_bitpack_to_bytes);
    SUITE_ADD_TEST(suite, test_bitpack_from_bytes);

    return suite;
}

int main(void)
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, bitpack_get_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    return 0;
}
