#ifndef _BITPACK_H
#define _BITPACK_H

/**
 * @file bitpack.h
 * @brief bitpack typedefs, defines, and exported function prototypes
 */

/** Bitpack success return value. */
#define BITPACK_RV_SUCCESS 1

/** Bitpack failure return value. */
#define BITPACK_RV_ERROR   0

/**
 * The number of bytes allocated by bitpack_init_default() to hold the
 * bitpack.
 */
#define BITPACK_DEFAULT_MEM_SIZE 32

/** The maximum size of a bitpack error string. */
#define BITPACK_ERR_BUF_SIZE 100

/** The various bitpack error types. */
typedef enum {
    BITPACK_ERR_CLEAR         = 0,
    BITPACK_ERR_MALLOC_FAILED = 1,
    BITPACK_ERR_INVALID_INDEX = 2,
    BITPACK_ERR_VALUE_TOO_BIG = 3,
    BITPACK_ERR_RANGE_TOO_BIG = 4,
    BITPACK_ERR_READ_PAST_END = 5,
    BITPACK_ERR_EMPTY         = 6
} bitpack_err_t;

struct _bitpack_t
{
    unsigned long  size;                            /** size of bitpack in bits */
    unsigned long  read_pos;                        /** current position for reading */
    unsigned long  data_size;                       /** amount of allocated memory */
    unsigned char *data;                            /** pointer to the acutal data */
    bitpack_err_t  error;                           /** error status of last operation */
    char           error_str[BITPACK_ERR_BUF_SIZE]; /** error string of last operation */
};

/** The Bitpack object type. */
typedef struct _bitpack_t *bitpack_t;

/**
 * @brief Default bitpack constructor.
 *
 * Allocates and returns a new bitpack object.  @c BITPACK_DEFAULT_MEM_SIZE bytes
 * are allocated to store the bits.  Use #bitpack_init() to control the
 * number of bytes allocated by the constructor.
 *
 * @return the newly allocated bitpack object
 */
#define bitpack_init_default() bitpack_init(BITPACK_DEFAULT_MEM_SIZE)

/**
 * @brief Bitpack constructor.
 *
 * Allocates and returns a new bitpack object.  The number of bytes allocated
 * to store the bits is specified by the num_bytes parameter.
 *
 * @param[in] num_bytes number of bytes to allocate for bit storage
 * @return the newly allocated bitpack object
 */
bitpack_t bitpack_init(unsigned long num_bytes);

/**
 * @brief Bitpack constructor.
 *
 * Allocates and returns a new bitpack object.  The contents of the bitpack
 * are initialized from an external byte array.  The contents of the external
 * byte array are copied into the bitpack object and are not modified.
 *
 * @param[in] bytes pointer to the external byte array
 * @param[in] num_bytes size of the external byte array
 * @return the newly allocated bitpack object
 */
bitpack_t bitpack_init_from_bytes(unsigned char *bytes, unsigned long num_bytes);

/**
 * @brief Bitpack destructor.
 *
 * Destroys a bitpack object, freeing all memory it contained.
 *
 * @param[in] bp the bitpack object
 */
void bitpack_destroy(bitpack_t bp);

/**
 * @brief Access the current size in bits of the bitpack object.
 *
 * @param[in] bp the bitpack object
 * @return the current size of the bitpack object
 */
unsigned long bitpack_size(bitpack_t bp);

/**
 * @brief Access the amount of data currently allocated to this bitpack object.
 *
 * @param[in] bp the bitpack object
 * @return the number of bytes allocated
 */
unsigned long bitpack_data_size(bitpack_t bp);

/**
 * @brief Access the current read position of the bitpack object.
 *
 * Returns the current postion in the bitpack object that the next call to
 * bitpack_read_bits() or bitpack_read_bytes() will start reading bits.
 *
 * @param[in] bp the bitpack object
 * @return the current read position
 */
unsigned long bitpack_read_pos(bitpack_t bp);

/**
 * @brief Reset the current read position to the beginning of the bitpack object.
 *
 * @param[in] bp the bitpack object
 */
void bitpack_reset_read_pos(bitpack_t bp);

/**
 * @brief Access the error type from a bitpack object.
 *
 * Returns the error type currently set in the bitpack object.  To get the
 * string representation of this error, see bitpack_get_error_str().
 *
 * @param[in] bp the bitpack object
 * @return the error type
 */
bitpack_err_t bitpack_get_error(bitpack_t bp);

/**
 * @brief Access the error string from a bitpack object.
 *
 * Returns a static character pointer containing the error string set in the
 * bitpack object.  This function should be called anytime a bitpack function
 * returns @c BITPACK_RV_ERROR.  The error status inside a bitpack object is
 * always reset when a subsequent bitpack function is called on the object.
 *
 * Note that the returned string is static and should NOT be passed to @c free().
 *
 * @param[in] bp the bitpack object
 * @return the static error string or @c NULL if the last operation on this
 * bitpack object did not fail
 */
char *bitpack_get_error_str(bitpack_t bp);

/**
 * @brief Turn on/set a particular bit in a bitpack object.
 *
 * Sets the bit at @c index.  If @c index is greater than the current size of the
 * bitpack object, then the size is expanded and the current append position
 * is set to this index.
 *
 * Returns @c BITPACK_RV_SUCCESS upon success.  Returns @c BITPACK_RV_ERROR upon
 * error and bitpack_get_error() can be used to find out why.
 *
 * @param[in] bp the bitpack object
 * @param[in] index the bit index to set
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_on(bitpack_t bp, unsigned long index);

/**
 * @brief Turn off/unset a particular bit in a bitpack object.
 *
 * Unsets the bit at @c index.  If @c index is greater than the current size of the
 * bitpack object, then the size is expanded and the current append position
 * is set to this index.
 *
 * @param[in] bp the bitpack object
 * @param[in] index the bit index to unset
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_off(bitpack_t bp, unsigned long index);

/**
 * @brief Access a particular bit.
 *
 * Get the value of the bit at @c index.
 *
 * @param[in]  bp the bitpack object
 * @param[in]  index the bit index to get
 * @param[out] bit value of the bit
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_get(bitpack_t bp, unsigned long index, unsigned char *bit);

/**
 * @brief Set the specified range of bits in a bitpack object.
 *
 * Packs @c value into @c num_bits bits starting at @c index.  The number of bits
 * required to represent @c value is checked against the size of the range.  If
 * @c index + @c num_bits is greater than the current size of the bitpack, then the
 * size is adjusted appropriately.
 *
 * @param[in] bp the bitpack object
 * @param[in] value the value to set
 * @param[in] num_bits the number of bits to pack the value into
 * @param[in] index the bit index to start packing value
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_set_bits(bitpack_t bp, unsigned long value, unsigned long num_bits, unsigned long index);

/**
 * @brief Set the specified range of bytes in a bitpack object.
 *
 * Packs the byte array @c value into @c num_bytes starting at @c index.  The size of
 * the bitpack object is adjust appropriately if necessary.
 *
 * @param[in] bp the bitpack object
 * @param[in] value the byte array to set
 * @param[in] num_bytes the number of bytes in @c value
 * @param[in] index the bit index to start packing @c value
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_set_bytes(bitpack_t bp, unsigned char *value, unsigned long num_bytes, unsigned long index);

/**
 * @brief Access the value of a range of bits.
 *
 * Unpacks @c num_bits bits at index @c index and sets the value to @c value.
 *
 * @param[in]  bp the bitpack object
 * @param[in]  num_bits the number of bits to unpack
 * @param[in]  index the bit index to start unpacking from
 * @param[out] value pointer to the location to write the value of the unpacked bits to
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_get_bits(bitpack_t bp, unsigned long num_bits, unsigned long index, unsigned long *value);

/**
 * @brief Access the value of a range of bytes.
 *
 * Unpacks @c num_bytes bytes at index @c index and sets the value to @c value.
 *
 * Allocates @c num_bytes bytes to write the unpacked bytes to and sets the
 * pointer pointed to by @c value to the unpacked bytes.  The unpacked bytes
 * should be freed by the caller.
 *
 * @param[in]  bp the bitpack object
 * @param[in]  num_bytes the number of bytes to unpack
 * @param[in]  index the bit index to start unpacking from
 * @param[out] value pointer to the location to write the unpacked byte array
 *             pointer to, will be set to @c NULL on failure
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_get_bytes(bitpack_t bp, unsigned long num_bytes, unsigned long index, unsigned char **value);

/**
 * @brief Append a particular value to the end of a bitpack object.
 *
 * Packs @c value into @c num_bits bits at the end of the bitpack object.  On
 * success, the size of the bitpack object is increased by @c num_bits.
 *
 * @param[in] bp the bitpack object
 * @param[in] value the value to set
 * @param[in] num_bits the number of bits to pack the value into
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
#define bitpack_append_bits(bp, value, num_bits) bitpack_set_bits(bp, value, num_bits, bitpack_size(bp))

/**
 * @brief Append the specified range of bytes to the end of a bitpack object.
 *
 * Packs the byte array @c value into @c num_bytes starting at then end of the
 * bitpack object.  On success, the size of the bitpack object is increased by
 * @c num_bytes * 8 bits.
 *
 * @param[in] bp the bitpack object
 * @param[in] value the byte array to set
 * @param[in] num_bytes the number of bytes in @c value
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
#define bitpack_append_bytes(bp, value, num_bytes) bitpack_set_bytes(bp, value, num_bytes, bitpack_size(bp))

/**
 * @brief Access the value of a range of bits at the current read position.
 *
 * Unpacks @c num_bits bits at the current read position (see bitpack_read_pos())
 * and sets the value to @c value.  The current read position is advanced by
 * @c num_bits bits.
 *
 * @param[in]  bp the bitpack object
 * @param[in]  num_bits the number of bits to unpack
 * @param[out] value pointer to the location to write the value of the unpacked bits to
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_read_bits(bitpack_t bp, unsigned long num_bits, unsigned long *value);

/**
 * @brief Access the value of a range of bytes at the current read position.
 *
 * Unpacks @c num_bytes bytes at the current read position (see bitpack_read_pos())
 * and sets the value to @c value.  The current read position is advanced by
 * @c num_bytes * 8 bits.
 *
 * The unpacked bytes are allocated on the heap and should be freed by the
 * caller.
 *
 * @param[in]  bp the bitpack object
 * @param[in]  num_bytes the number of bytes to unpack
 * @param[out] value pointer to the location to write the unpacked byte array
 *             pointer to, will be set to @c NULL on failure
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_read_bytes(bitpack_t bp, unsigned long num_bytes, unsigned char **value);

/**
 * @brief Convert the bitpack object to a string of 1s and 0s.
 *
 * Converts the bitpack object to its binary representation.
 *
 * The output string @c str is allocated on the heap and should be freed by the
 * caller.
 *
 * @param[in]  bp the bitpack object
 * @param[out] str pointer to the location to write the binary string to
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_to_bin(bitpack_t bp, char **str);

/**
 * @brief Convert the bitpack object to a byte array.
 *
 * Converts the bitpack object to an array of bytes.  If the current size of
 * the bitpack object is not a multiple of 8, the last byte in the returned
 * byte array will be padded with the appropriate number of 0 bits.
 *
 * The output string @c bytes is allocated on the heap and should be freed by the
 * caller.
 *
 * The number of bytes returned is the current size in bits of the bitpack,
 * divided by 8 and rounded up to the nearest byte.  The output parameter
 * @c num_bytes will tell you the exact value.
 *
 * @param[in]  bp the bitpack object
 * @param[out] value pointer to the location to write the byte array to
 * @param[out] num_bytes pointer to the location to write the number of bytes returned
 * @return @c BITPACK_RV_SUCCESS on success, @c BITPACK_RV_ERROR on failure
 */
int bitpack_to_bytes(bitpack_t bp, unsigned char **value, unsigned long *num_bytes);

#endif

