#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitpack.h"

/* round up to the nearest multiple of 8 */
static unsigned long round8(unsigned long v)
{
    if (v % 8 != 0) {
        v += 8 - (v % 8);
    }

    return v;
}

/* clear any previous errors on a bitpack object */
static void _bitpack_err_clear(bitpack_t bp)
{
    if (bp->error != BITPACK_ERR_CLEAR) {
        bp->error = BITPACK_ERR_CLEAR;
        memset(bp->error_str, '\0', BITPACK_ERR_BUF_SIZE);
    }
}

/* increase the size of a bitpack object, allocating more memory if necessary */
static int _bitpack_resize(bitpack_t bp, unsigned long new_size)
{
   unsigned long new_data_size = round8(new_size) / 8;

   if (new_data_size > bp->data_size) {
        bp->data = realloc(bp->data, new_data_size);

        if (bp->data == NULL) {
            bp->error = BITPACK_ERR_MALLOC_FAILED;
            strncpy(bp->error_str, "memory allocation failed", BITPACK_ERR_BUF_SIZE);
            return BITPACK_RV_ERROR;
        }

        memset(bp->data + bp->data_size, 0, new_data_size - bp->data_size);

        bp->data_size = new_data_size;
   }

   bp->size = new_size;

   return BITPACK_RV_SUCCESS;
}

bitpack_t bitpack_init(unsigned long num_bytes)
{
    bitpack_t      bp;
    unsigned char *data;

    bp = malloc(sizeof(struct _bitpack_t));
    if (bp == NULL) return NULL;

    data = malloc(num_bytes);

    if (data == NULL) {
        free(bp);
        return NULL;
    }

    memset(data, 0, num_bytes);

    bp->size      = 0;
    bp->read_pos  = 0;
    bp->data_size = num_bytes;
    bp->data      = data;
    bp->error     = BITPACK_ERR_CLEAR;
    memset(bp->error_str, '\0', BITPACK_ERR_BUF_SIZE);

    return bp;
}

bitpack_t bitpack_init_from_bytes(unsigned char *bytes, unsigned long num_bytes)
{
    bitpack_t bp;

    bp = bitpack_init(num_bytes);

    if (bp == NULL) {
        return NULL;
    }

    memcpy(bp->data, bytes, num_bytes);
    bp->size = num_bytes * 8;

    return bp;
}

void bitpack_destroy(bitpack_t bp)
{
    free(bp->data);
    free(bp);
}

unsigned long bitpack_size(bitpack_t bp)
{
    return bp->size;
}

unsigned long bitpack_data_size(bitpack_t bp)
{
    return bp->data_size;
}

unsigned long bitpack_read_pos(bitpack_t bp)
{
    return bp->read_pos;
}

void bitpack_reset_read_pos(bitpack_t bp)
{
    bp->read_pos = 0;
}

bitpack_err_t bitpack_get_error(bitpack_t bp)
{
    return bp->error;
}

char *bitpack_get_error_str(bitpack_t bp)
{
    return bp->error_str;
}

int bitpack_on(bitpack_t bp, unsigned long index)
{
    unsigned long byte_offset;
    unsigned long bit_offset;

    _bitpack_err_clear(bp);

    if (bitpack_size(bp) == 0 || index > bitpack_size(bp) - 1) {
        if (!_bitpack_resize(bp, index + 1)) {
            return BITPACK_RV_ERROR;
        }
    }

    byte_offset = index / 8;
    bit_offset  = index % 8;

    bp->data[byte_offset] |= (0x80 >> bit_offset);

    return BITPACK_RV_SUCCESS;
}

int bitpack_off(bitpack_t bp, unsigned long index)
{
    unsigned long byte_offset;
    unsigned long bit_offset;

    _bitpack_err_clear(bp);

    if (bitpack_size(bp) == 0 || index > bitpack_size(bp) - 1) {
        if (!_bitpack_resize(bp, index + 1)) {
            return BITPACK_RV_ERROR;
        }
    }

    byte_offset = index / 8;
    bit_offset  = index % 8;

    bp->data[byte_offset] &= ~(0x80 >> bit_offset);

    return BITPACK_RV_SUCCESS;
}

int bitpack_get(bitpack_t bp, unsigned long index, unsigned char *bit)
{
    unsigned long byte_offset;
    unsigned long bit_offset;

    _bitpack_err_clear(bp);

    if (bitpack_size(bp) == 0) {
        bp->error = BITPACK_ERR_EMPTY;
        strncpy(bp->error_str, "bitpack is empty", BITPACK_ERR_BUF_SIZE);
        return BITPACK_RV_ERROR;
    }

    if (index > bitpack_size(bp) - 1) {
        bp->error = BITPACK_ERR_INVALID_INDEX;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "invalid index (%lu), max index is %lu",
                index, bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    byte_offset = index / 8;
    bit_offset  = index % 8;

    if (bp->data[byte_offset] & (0x80 >> bit_offset)) {
        *bit = 1;
    }
    else {
        *bit = 0;
    }

    return BITPACK_RV_SUCCESS;
}

int bitpack_set_bits(bitpack_t bp, unsigned long value, unsigned long num_bits, unsigned long index)
{
    unsigned long i;
    unsigned long mask;

    _bitpack_err_clear(bp);

    /* make sure the range isn't bigger than the size of an unsigned long */
    if (num_bits > sizeof(unsigned long) * 8) {
        bp->error = BITPACK_ERR_RANGE_TOO_BIG;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "range size %lu bits is too large (maximum size is %lu bits)",
                num_bits, sizeof(unsigned long) * 8);
        return BITPACK_RV_ERROR;
    }

    /* make sure that the range is large enough to pack value */
    if (value > pow(2, num_bits) - 1) {
        bp->error = BITPACK_ERR_VALUE_TOO_BIG;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "value %lu does not fit in %lu bits",
                value, num_bits);
        return BITPACK_RV_ERROR;
    }

    if (bitpack_size(bp) < index + num_bits) {
        if (!_bitpack_resize(bp, index + num_bits)) {
            return BITPACK_RV_ERROR;
        }
    }

    for (i = num_bits; i != 0; i--) {
        mask = 1 << (i - 1);
        if (value & mask) {
            if (!bitpack_on(bp, index)) {
                return BITPACK_RV_ERROR;
            }
        }
        else {
            if (!bitpack_off(bp, index)) {
                return BITPACK_RV_ERROR;
            }
        }
        index++;
    }

    return BITPACK_RV_SUCCESS;
}

int bitpack_set_bytes(bitpack_t bp, unsigned char *value, unsigned long num_bytes, unsigned long index)
{
    unsigned long i;

    _bitpack_err_clear(bp);

    if (bitpack_size(bp) < index + num_bytes * 8) {
        if (!_bitpack_resize(bp, index + num_bytes * 8)) {
            return BITPACK_RV_ERROR;
        }
    }

    if (index % 8 == 0) {
        /* index is at the beginning of a byte, so just do a memcpy */
        memcpy(bp->data + index / 8, value, num_bytes);
    }
    else {
        /* need to set each bit individually */
        for (i = 0; i < num_bytes; i++) {
            if (!bitpack_set_bits(bp, value[i], 8, index + i * 8)) {
                return BITPACK_RV_ERROR;
            }
        }
    }

    return BITPACK_RV_SUCCESS;
}

int bitpack_get_bits(bitpack_t bp, unsigned long num_bits, unsigned long index, unsigned long *value)
{
    unsigned long i, v = 0;
    unsigned char bit;

    _bitpack_err_clear(bp);

    if (index >= bitpack_size(bp)) {
        bp->error = BITPACK_ERR_INVALID_INDEX;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "invalid index (%lu), max index is %lu",
                index, bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    if (index + num_bits > bitpack_size(bp)) {
        bp->error = BITPACK_ERR_READ_PAST_END;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "attempted to read past end of bitpack (last index is %lu)",
                bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    if (num_bits > sizeof(unsigned long) * 8) {
        bp->error = BITPACK_ERR_RANGE_TOO_BIG;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "range size %lu bits is too large (maximum size is %lu bits)",
                num_bits, sizeof(unsigned long) * 8);
        return BITPACK_RV_ERROR;
    }

    for (i = 0; i < num_bits; i++) {
        bitpack_get(bp, index + i, &bit);

        if (bit == 1) {
            v |= bit << (num_bits - i - 1);
        }
    }

    *value = v;

    return BITPACK_RV_SUCCESS;
}

int bitpack_get_bytes(bitpack_t bp, unsigned long num_bytes, unsigned long index, unsigned char **value)
{
    unsigned long  i;
    unsigned long  byte;
    unsigned char *unpacked;

    _bitpack_err_clear(bp);

    if (index >= bitpack_size(bp)) {
        bp->error = BITPACK_ERR_INVALID_INDEX;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "invalid index (%lu), max index is %lu",
                index, bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    if (index + num_bytes * 8 > bitpack_size(bp)) {
        bp->error = BITPACK_ERR_READ_PAST_END;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "attempted to read past end of bitpack (last index is %lu)",
                bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    unpacked = malloc(num_bytes);
    if (unpacked == NULL) {
        bp->error = BITPACK_ERR_MALLOC_FAILED;
        strncpy(bp->error_str, "memory allocation failed", BITPACK_ERR_BUF_SIZE);
        return BITPACK_RV_ERROR;
    }

    if (index % 8 == 0) {
        /* index is the start of a byte, so just do a memcpy */
        memcpy(unpacked, bp->data + index / 8, num_bytes);
    }
    else {
        /* need to unpack a byte at a time */
        for (i = 0; i < num_bytes; i++) {
            bitpack_get_bits(bp, 8, index + i * 8, &byte);
            unpacked[i] = byte;
        }
    }

    *value = unpacked;

    return BITPACK_RV_SUCCESS;
}

int bitpack_read_bits(bitpack_t bp, unsigned long num_bits, unsigned long *value)
{
    int rv;

    _bitpack_err_clear(bp);

    if (bp->read_pos + num_bits > bitpack_size(bp)) {
        bp->error = BITPACK_ERR_READ_PAST_END;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "attempted to read past end of bitpack (last index is %lu)",
                bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    rv = bitpack_get_bits(bp, num_bits, bp->read_pos, value);

    if (rv == BITPACK_RV_SUCCESS) {
        bp->read_pos += num_bits;
    }
    else {
        return rv;
    }

    return BITPACK_RV_SUCCESS;
}

int bitpack_read_bytes(bitpack_t bp, unsigned long num_bytes, unsigned char **value)
{
    int rv;

    _bitpack_err_clear(bp);

    if (bp->read_pos + num_bytes * 8 > bitpack_size(bp)) {
        bp->error = BITPACK_ERR_READ_PAST_END;
        snprintf(bp->error_str, BITPACK_ERR_BUF_SIZE,
                "attempted to read past end of bitpack (last index is %lu)",
                bitpack_size(bp) - 1);
        return BITPACK_RV_ERROR;
    }

    rv = bitpack_get_bytes(bp, num_bytes, bp->read_pos, value);

    if (rv == BITPACK_RV_SUCCESS) {
        bp->read_pos += num_bytes * 8;
    }
    else {
        return rv;
    }

    return BITPACK_RV_SUCCESS;
}

int bitpack_to_bin(bitpack_t bp, char **str)
{
    unsigned long  i;
    char          *string;
    char           tmp[3];
    unsigned char  bit;

    _bitpack_err_clear(bp);

    string = malloc(bitpack_size(bp) + 1);

    if (string == NULL) {
        *str = NULL;
        bp->error = BITPACK_ERR_MALLOC_FAILED;
        strncpy(bp->error_str, "memory allocation failed", BITPACK_ERR_BUF_SIZE);
        return BITPACK_RV_ERROR;
    }

    string[0] = '\0';

    for (i = 0; i < bitpack_size(bp); i++) {
        bitpack_get(bp, i, &bit);
        sprintf(tmp, "%d", bit);
        strncat(string, tmp, 1); 
    }

    *str = string;

    return BITPACK_RV_SUCCESS;
}

int bitpack_to_bytes(bitpack_t bp, unsigned char **value, unsigned long *num_bytes)
{
    unsigned char *bytes;
    unsigned long  bytes_size = round8(bp->size) / 8;

    _bitpack_err_clear(bp);

    bytes = malloc(bytes_size);

    if (bytes == NULL) {
        bp->error = BITPACK_ERR_MALLOC_FAILED;
        strncpy(bp->error_str, "memory allocation failed", BITPACK_ERR_BUF_SIZE);
        return BITPACK_RV_ERROR;
    }

    memcpy(bytes, bp->data, bytes_size);

    *value = bytes;

    if (num_bytes) {
        *num_bytes = bytes_size;
    }

    return BITPACK_RV_SUCCESS;
}

