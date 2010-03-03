#include "ruby.h"
#include "string.h"
#include "bitpack.h"

 /* the BitPack class object */
static VALUE cBitPack;

/* mapping of BitPack error codes to ruby exceptions */
static VALUE bp_exceptions[6];

/*
 * call-seq:
 *   BitPack.new    -> a new BitPack object
 *   BitPack.new(n) -> a new BitPack object
 *
 * Creates a new BitPack object.  The number of bytes used internally
 * to store the bit string can optionally be set to +n+.
 *
 */
static VALUE bp_new(int argc, VALUE *argv, VALUE class)
{
    VALUE     bp_obj;
    bitpack_t bp;

    if (argc == 0) {
        bp = bitpack_init_default();
    }
    else {
        bp = bitpack_init(NUM2ULONG(argv[0]));
    }

    if (bp == NULL) {
        rb_raise(bp_exceptions[BITPACK_ERR_MALLOC_FAILED], "malloc() failed");
    }

    bp_obj = Data_Wrap_Struct(class, 0, bitpack_destroy, bp);

    return bp_obj;
}

/*
 * call-seq:
 *   BitPack.from_bytes(string) -> a new BitPack object
 *
 * Creates a new BitPack object that is initialzed with the contents
 * of +string+.
 *
 * === Example
 *
 *   >> bp = BitPack.from_bytes("ruby")
 *   => 01110010011101010110001001111001
 *   >> 4.times { p bp.read_bits(8).chr }
 *   "r"
 *   "u"
 *   "b"
 *   "y"
 *   => 4
 */
static VALUE bp_from_bytes(VALUE class, VALUE bytes_str)
{
    VALUE          bp_obj;
    VALUE          str;;
    bitpack_t      bp;

    str = StringValue(bytes_str);

    bp = bitpack_init_from_bytes((unsigned char *)RSTRING(str)->ptr, RSTRING(str)->len);

    if (bp == NULL) {
        rb_raise(bp_exceptions[BITPACK_ERR_MALLOC_FAILED], "malloc() failed");
    }

    bp_obj = Data_Wrap_Struct(class, 0, bitpack_destroy, bp);

    return bp_obj;
}

/*
 * call-seq:
 *   bp.size -> Integer
 *
 * Access the current size of the BitPack object in bits.
 */
static VALUE bp_size(VALUE self)
{
    bitpack_t     bp;
    unsigned long size;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    size = bitpack_size(bp);

    return ULONG2NUM(size);
}

/*
 * call-seq:
 *   bp.data_size -> Integer
 *
 * Access the number of bytes of memory currently allocated to this
 * BitPack object.
 */
static VALUE bp_data_size(VALUE self)
{
    bitpack_t     bp;
    unsigned long data_size;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    data_size = bitpack_data_size(bp);

    return ULONG2NUM(data_size);
}

/*
 * call-seq:
 *   bp.read_pos -> Integer
 *
 * Access the current read position of this BitPack object.
 *
 * === Example
 *
 *   >> bp = BitPack.from_bytes("test")
 *   => 01110100011001010111001101110100
 *   >> bp.read_pos
 *   => 0
 *   >> bp.read_bits(8)
 *   => 116
 *   >> bp.read_pos
 *   => 8
 */
static VALUE bp_read_pos(VALUE self)
{
    bitpack_t     bp;
    unsigned long read_pos;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    read_pos = bitpack_read_pos(bp);

    return ULONG2NUM(read_pos);
}

/*
 * call-seq:
 *   bp.reset_read_pos
 *
 * Reset the current read position to the beginning of this BitPack
 * object.
 */
static VALUE bp_reset_read_pos(VALUE self)
{
    bitpack_t bp;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    bitpack_reset_read_pos(bp);

    return self;
}

/*
 * call-seq:
 *   bp.on(i)
 *
 * Sets the bit at index +i+.  If +i+ is greater than the current
 * size of the BitPack object, then the size is expanded and the
 * current append position is set to this index.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.on(0)
 *   => 1
 *   >> bp.on(2)
 *   => 101
 *   >> bp.on(7)
 *   => 10100001
 *   >> bp.on(6)
 *   => 10100011
 */
static VALUE bp_on(VALUE self, VALUE index)
{
    bitpack_t bp;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_on(bp, NUM2ULONG(index))) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.off(i)
 *
 * Unsets the bit at index +i+.  If +i+ is greater than the current
 * size of the BitPack object, then the size is expanded and the
 * current append position is set to this index.
 */
static VALUE bp_off(VALUE self, VALUE index)
{
    bitpack_t bp;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_off(bp, NUM2ULONG(index))) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.get(i) -> 0 or 1
 *
 * Access the value of the bit at index +i+.
 */
static VALUE bp_get(VALUE self, VALUE index)
{
    bitpack_t     bp;
    unsigned char bit;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_get(bp, NUM2ULONG(index), &bit)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return INT2FIX(bit);
}

/*
 * call-seq:
 *   bp.set_bits(value, num_bits, i) -> self
 *
 * Sets the specified range of bits in a BitPack object.
 *
 * Packs the Integer +value+ into +num_bits+ bits starting at index
 * +i+.  The number of bits required to represent +value+ is checked
 * against the size of the range.  If <tt>i + num_bits</tt> is greater
 * than the current size of the BitPack, then the size is adjusted
 * appropriately.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.set_bits(0xff, 8, 0)
 *   => 11111111
 *   >> bp.set_bits(0xaa, 8, 8)
 *   => 1111111110101010
 */
static VALUE bp_set_bits(VALUE self, VALUE value, VALUE num_bits, VALUE index)
{
    bitpack_t bp;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_set_bits(bp, NUM2ULONG(value), NUM2ULONG(num_bits), NUM2ULONG(index))) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.set_bytes(string, i)
 *
 * Sets the specified range of bytes in a BitPack object.
 *
 * Packs +string+ into the BitPack object starting at index +i+.  The
 * size of the BitPack object is adjusted appropriately if necessary.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.set_bytes("ruby", 0)
 *   => 01110010011101010110001001111001
 *   >> 4.times { p bp.read_bits(8).chr }
 *   "r"
 *   "u"
 *   "b"
 *   "y"
 *   => 4
 */
static VALUE bp_set_bytes(VALUE self, VALUE bytes, VALUE index)
{
    bitpack_t bp;
    VALUE     str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    str = StringValue(bytes);

    if (!bitpack_set_bytes(bp, (unsigned char *)RSTRING(str)->ptr,
          RSTRING(str)->len, NUM2ULONG(index))) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.get_bits(num_bits, i) -> Integer
 *
 * Access the value stored in a range of bits starting at index +i+.
 *
 * Unpacks +num_bits+ starting from index +i+ and returns the Integer
 * value.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.append_bits(1, 4)
 *   => 0001
 *   >> bp.append_bits(2, 4)
 *   => 00010010
 *   >> bp.append_bits(3, 4)
 *   => 000100100011
 *   >> bp.append_bits(4, 4)
 *   => 0001001000110100
 *   >> bp.get_bits(4, 0)
 *   => 1
 *   >> bp.get_bits(4, 4)
 *   => 2
 *   >> bp.get_bits(4, 8)
 *   => 3
 *   >> bp.get_bits(4, 12)
 *   => 4
 */
static VALUE bp_get_bits(VALUE self, VALUE num_bits, VALUE index)
{
    bitpack_t     bp;
    unsigned long value;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_get_bits(bp, NUM2ULONG(num_bits), NUM2ULONG(index), &value)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return ULONG2NUM(value);
}

/*
 * call-seq:
 *   bp.get_bytes(num_bytes, i) -> String
 *
 * Access the value stored in a range of bytes starting at index +i+.
 *
 * Unpacks +num_bytes+ starting from bit index +i+ and returns the String
 * value.
 *
 * === Example
 *
 *   >> bp =  BitPack.from_bytes("foobar")
 *   => 011001100110111101101111011000100110000101110010
 *   >> bp.get_bytes(3, 24)
 *   => "bar"
 */
static VALUE bp_get_bytes(VALUE self, VALUE num_bytes, VALUE index)
{
    bitpack_t      bp;
    unsigned char *bytes;
    VALUE          str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_get_bytes(bp, NUM2ULONG(num_bytes), NUM2ULONG(index), &bytes)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    str = rb_str_new((char *)bytes, NUM2ULONG(num_bytes));

    free(bytes);

    return str;
}

/*
 * call-seq:
 *   bp.append_bits(value, num_bits)
 *
 * Append the Integer +value+ to the end of a BitPack object.
 *
 * Packs +num_bits+ bits at the end of a BitPack object.  The size of
 * the BitPack object is increased by +num_bits+ as a result.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.append_bits(1, 3)
 *   => 001
 *   >> bp.append_bits(3, 3)
 *   => 001011
 *   >> bp.append_bits(7, 3)
 *   => 001011111
 */
static VALUE bp_append_bits(VALUE self, VALUE value, VALUE num_bits)
{
    bitpack_t bp;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_append_bits(bp, NUM2ULONG(value), NUM2ULONG(num_bits))) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.append_bytes(string)
 *
 * Append +string+ to the end of a BitPack object.
 *
 * Packs +string+ at the end of a BitPack object.  The size of the
 * BitPack object is increased by the length of +string+ as a result.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.append_bytes("bit")
 *   => 011000100110100101110100
 *   >> bp.append_bytes("pack")
 *   => 01100010011010010111010001110000011000010110001101101011
 *   >> 7.times { p bp.read_bits(8).chr }
 *   "b"
 *   "i"
 *   "t"
 *   "p"
 *   "a"
 *   "c"
 *   "k"
 *   => 7
 */
static VALUE bp_append_bytes(VALUE self, VALUE value)
{
    bitpack_t bp;
    VALUE     str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    str = StringValue(value);

    if (!bitpack_append_bytes(bp, (unsigned char *)RSTRING(str)->ptr,
          RSTRING(str)->len)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return self;
}

/*
 * call-seq:
 *   bp.read_bits(num_bits) -> Integer
 *
 * Access the value of a range of bits at the current read position.
 *
 * Unpacks +num_bits+ bits starting at the current read position (see
 * Bitpack#read_pos) and returns the integer value.  The current read
 * position is advanced by +num_bits+ bits.
 *
 * === Example
 *
 *   >> bp = BitPack.from_bytes("ruby")
 *   => 01110010011101010110001001111001
 *   >> 4.times { p bp.read_bits(8).chr }
 *   "r"
 *   "u"
 *   "b"
 *   "y"
 *   => 4
 */
static VALUE bp_read_bits(VALUE self, VALUE num_bits)
{
    bitpack_t     bp;
    unsigned long value;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_read_bits(bp, NUM2ULONG(num_bits), &value)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    return ULONG2NUM(value);
}

/*
 * call-seq:
 *   bp.read_bytes(num_bytes) -> String
 *
 * Access the value of a range of bytes at the current read position.
 *
 * Unpacks +num_bytes+ bytes starting at the current read position (see
 * Bitpack#read_pos) and returns the String value.  The current read
 * position is advanced by <tt>num_bytes * 8</tt> bits.
 *
 * === Example
 *
 *   >> bp = BitPack.from_bytes("foobar")
 *   => 011001100110111101101111011000100110000101110010
 *   >> bp.read_bytes(3)
 *   => "foo"
 *   >> bp.read_bytes(3)
 *   => "bar"
 */
static VALUE bp_read_bytes(VALUE self, VALUE num_bytes)
{
    bitpack_t      bp;
    unsigned char *value;
    VALUE          str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_read_bytes(bp, NUM2ULONG(num_bytes), &value)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    str = rb_str_new((char *)value, NUM2ULONG(num_bytes));

    free(value);

    return str;
}

/*
 * call-seq:
 *   bp.to_bin -> String
 *
 * Converts the BitPack object to a string of 1s and 0s.
 */
static VALUE bp_to_bin(VALUE self)
{
    bitpack_t  bp;
    char      *s;
    VALUE      str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_to_bin(bp, &s)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    str = rb_str_new2(s);

    free(s);

    return str;
}

/*
 * call-seq:
 *   bp.to_bytes -> String
 *
 * Converts the BitPack object to a string of bytes.  If the current
 * size of the BitPack object is not a multiple of 8, the last byte
 * in the returned String will be padded with the appropriate number
 * of 0 bits.
 *
 * === Example
 *
 *   >> bp = BitPack.new
 *   => 
 *   >> bp.append_bits(?r, 8)
 *   => 01110010
 *   >> bp.append_bits(?u, 8)
 *   => 0111001001110101
 *   >> bp.append_bits(?b, 8)
 *   => 011100100111010101100010
 *   >> bp.append_bits(?y, 8)
 *   => 01110010011101010110001001111001
 *   >> bp.to_bytes
 *   => "ruby"
 */
static VALUE bp_to_bytes(VALUE self)
{
    bitpack_t      bp;
    unsigned char *s;
    unsigned long  num_bytes;
    VALUE          str;

    Data_Get_Struct(self, struct _bitpack_t, bp);

    if (!bitpack_to_bytes(bp, &s, &num_bytes)) {
        rb_raise(bp_exceptions[bitpack_get_error(bp)],
                bitpack_get_error_str(bp));
    }

    str = rb_str_new((char *)s, num_bytes);

    free(s);

    return str;
}

/*
 * A library for easily packing and unpacking binary strings with fields of
 * arbitrary bit lengths.
 */
void Init_bitpack()
{
    cBitPack = rb_define_class("BitPack", rb_cObject);

    rb_define_singleton_method(cBitPack, "new",        bp_new,        -1);
    rb_define_singleton_method(cBitPack, "from_bytes", bp_from_bytes,  1);

    rb_define_method(cBitPack, "size",            bp_size,             0);
    rb_define_method(cBitPack, "data_size",       bp_data_size,        0);
    rb_define_method(cBitPack, "read_pos",        bp_read_pos,         0);
    rb_define_method(cBitPack, "reset_read_pos",  bp_reset_read_pos,   0);
    rb_define_method(cBitPack, "on",              bp_on,               1);
    rb_define_method(cBitPack, "off",             bp_off,              1);
    rb_define_method(cBitPack, "get",             bp_get,              1);
    rb_define_method(cBitPack, "[]",              bp_get,              1);
    rb_define_method(cBitPack, "set_bits",        bp_set_bits,         3);
    rb_define_method(cBitPack, "get_bits",        bp_get_bits,         2);
    rb_define_method(cBitPack, "set_bytes",       bp_set_bytes,        2);
    rb_define_method(cBitPack, "get_bytes",       bp_get_bytes,        2);
    rb_define_method(cBitPack, "append_bits",     bp_append_bits,      2);
    rb_define_method(cBitPack, "append_bytes",    bp_append_bytes,     1);
    rb_define_method(cBitPack, "read_bits",       bp_read_bits,        1);
    rb_define_method(cBitPack, "read_bytes",      bp_read_bytes,       1);
    rb_define_method(cBitPack, "to_bin",          bp_to_bin,           0);
    rb_define_method(cBitPack, "to_s",            bp_to_bin,           0);
    rb_define_method(cBitPack, "to_bytes",        bp_to_bytes,         0);

    bp_exceptions[BITPACK_ERR_MALLOC_FAILED] = rb_eNoMemError;
    bp_exceptions[BITPACK_ERR_INVALID_INDEX] = rb_eRangeError;
    bp_exceptions[BITPACK_ERR_VALUE_TOO_BIG] = rb_eArgError;
    bp_exceptions[BITPACK_ERR_RANGE_TOO_BIG] = rb_eRangeError;
    bp_exceptions[BITPACK_ERR_READ_PAST_END] = rb_eRangeError;
    bp_exceptions[BITPACK_ERR_EMPTY]         = rb_eRangeError;

    /* require the pure ruby methods */
    rb_require("lib/bitpack.rb");
}

