
$:.unshift File.join(File.dirname(__FILE__), '..', 'ext')
require 'bitpack'

require 'test/unit'

class TC_BitPack < Test::Unit::TestCase
  def test_constructor
    bytes = [0xab, 0xcd, 0xef, 0x12].pack("C*")

    bp1 = BitPack.new
    assert_equal("", bp1.to_bin)
    assert_equal(0, bp1.size)

    bp2 = BitPack.from_bytes(bytes)
    assert_equal("10101011110011011110111100010010", bp2.to_bin)
    assert_equal(32, bp2.size)
  end

  def test_get_on_off
    bp = BitPack.new

    assert_equal(0, bp.size)
    bp.on(0)
    assert_equal(1, bp.size);
    bp.off(1)
    assert_equal(2, bp.size);
    bp.on(2)
    assert_equal(3, bp.size);
    bp.off(3)
    assert_equal(4, bp.size);
    bp.on(4)
    assert_equal(5, bp.size);
    bp.off(5)
    assert_equal(6, bp.size);
    bp.on(6)
    assert_equal(7, bp.size);
    bp.off(7)
    assert_equal(8, bp.size);
    bp.on(8)
    assert_equal(9, bp.size);
    bp.off(9)
    assert_equal(10, bp.size);

    assert_equal("1010101010", bp.to_bin);

    0.upto(9) do |i|
      expected_bit = (i % 2 == 0) ? 1 : 0;
      assert_equal(expected_bit, bp.get(i))
    end

    bp.off(0)
    bp.off(2)
    bp.off(4)
    bp.off(6)
    bp.off(8)

    assert_equal("0000000000", bp.to_bin);

    # error cases
    assert_raise RangeError, "invalid index (10), max index is 9" do
      bp.get(10)
    end

    bp = BitPack.new
    assert_raise RangeError, "bitpack is empty" do
      bp.get(10)
    end
  end

  def test_get_set_bits
    bp = BitPack.new(4)
    assert_equal(0, bp.size);
    assert_equal(4, bp.data_size);

    bp.set_bits(0xff, 8, 0)
    assert_equal(8, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("11111111", bp.to_bin)
    assert_equal(0xff, bp.get_bits(8, 0))

    bp.set_bits(5, 3, 8)
    assert_equal(11, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("11111111101", bp.to_bin)
    assert_equal(5, bp.get_bits(3, 8))

    bp.set_bits(21, 5, 11)
    assert_equal(16, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("1111111110110101", bp.to_bin)
    assert_equal(21, bp.get_bits(5, 11))

    bp.set_bits(0xffffffff, 32, 16)
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("111111111011010111111111111111111111111111111111", bp.to_bin)
    assert_equal(0xffffffff, bp.get_bits(32, 16))

    bp.set_bits(0, 8, 0)
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("000000001011010111111111111111111111111111111111", bp.to_bin)
    assert_equal(0, bp.get_bits(8, 0))

    unsigned_long_size = [1].pack("L").size

    # error cases
    msg = sprintf("range size %d bits is too large (maximum size is %d bits)", unsigned_long_size * 8 + 1, unsigned_long_size * 8)
    assert_raise RangeError, msg do
      bp.set_bits(0, unsigned_long_size * 8 + 1, 0)
    end

    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("000000001011010111111111111111111111111111111111", bp.to_bin)
    
    assert_raise ArgumentError, "value 8 does not fit in 3 bits" do
      bp.set_bits(8, 3, 0)
    end
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("000000001011010111111111111111111111111111111111", bp.to_bin)

    assert_raise RangeError, "invalid index (48), max index is 47" do
      bp.get_bits(3, 48)
    end

    assert_raise RangeError, "attempted to read past end of bitpack (last index is 47)" do
      bp.get_bits(25, 24)
    end

    bp.set_bits(0xffffffff, 32, 48)
    bp.set_bits(0xffffffff, 32, 80)
    bp.set_bits(0xffffffff, 32, 112)

    msg = sprintf("range size %d bits is too large (maximum size is %d bits)", unsigned_long_size * 8 + 1,
                  unsigned_long_size * 8)
    assert_raise RangeError, msg do
      bp.get_bits(unsigned_long_size * 8 + 1, 0)
    end
  end

  def test_get_set_bytes
    test_bytes1 = [ 0x01, 0x02, 0x03 ].pack("C*")
    test_bytes2 = [ 0xff, 0xfe, 0xfd ].pack("C*")
    test_bytes3 = [ 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff ].pack("C*")

    bp = BitPack.new(4)

    assert_equal(0, bp.size)
    assert_equal(4, bp.data_size)

    bp.set_bytes(test_bytes1, 0)
    assert_equal(24, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("000000010000001000000011", bp.to_bin)

    assert_equal(test_bytes1, bp.get_bytes(3, 0));

    bp.set_bytes(test_bytes2, 24)
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("000000010000001000000011111111111111111011111101", bp.to_bin)
    assert_equal(test_bytes2, bp.get_bytes(3, 24))

    # non-byte aligned set
    bp.set_bytes(test_bytes3, 50)
    assert_equal(98, bp.size)
    assert_equal(13, bp.data_size)
    assert_equal("00000001000000100000001111111111111111101111110100101010101011101111001100110111011110111011111111", bp.to_bin);
    assert_equal(test_bytes3, bp.get_bytes(6, 50))

    # error cases
    assert_raise RangeError, "invalid index(98), max index is 97" do
      bp.get_bytes(5, 98)
    end
    
    assert_raise RangeError, "attempted to read past end of bitpack (last index is 97)" do
      bp.get_bytes(13, 0)
    end
  end

  def test_append_bits
    bp = BitPack.new(4);

    assert_equal(0, bp.size)
    assert_equal(4, bp.data_size)

    bp.append_bits(0xff, 8)
    assert_equal(8, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("11111111", bp.to_bin);

    bp.append_bits(5, 3)
    assert_equal(11, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("11111111101", bp.to_bin)

    bp.append_bits(21, 5)
    assert_equal(16, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("1111111110110101", bp.to_bin)

    bp.append_bits(0xffffffff, 32)
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("111111111011010111111111111111111111111111111111", bp.to_bin);
  end

  def test_append_bytes
    test_bytes1 = [ 0x01, 0x02, 0x03 ].pack("C*")
    test_bytes2 = [ 0xff, 0xfe, 0xfd ].pack("C*")
    test_bytes3 = [ 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff ].pack("C*")

    bp = BitPack.new(4);

    assert_equal(0, bp.size)
    assert_equal(4, bp.data_size)

    bp.append_bytes(test_bytes1)
    assert_equal(24, bp.size)
    assert_equal(4, bp.data_size)
    assert_equal("000000010000001000000011", bp.to_bin);

    bp.append_bytes(test_bytes2)
    assert_equal(48, bp.size)
    assert_equal(6, bp.data_size)
    assert_equal("000000010000001000000011111111111111111011111101", bp.to_bin)

    # non-byte aligned set
    bp.append_bits(0, 2)
    bp.append_bytes(test_bytes3)
    assert_equal(98, bp.size)
    assert_equal(13, bp.data_size)
    assert_equal("00000001000000100000001111111111111111101111110100101010101011101111001100110111011110111011111111", bp.to_bin);
  end

  def test_read_bits
    bp = BitPack.new(4);

    bp.append_bits(0xff, 8);
    bp.append_bits(5, 3);
    bp.append_bits(21, 5);
    bp.append_bits(0xffffffff, 32);

    assert_equal(0, bp.read_pos)
    assert_equal(0xff, bp.read_bits(8))
    assert_equal(8, bp.read_pos)
    assert_equal(5, bp.read_bits(3))
    assert_equal(11, bp.read_pos)
    assert_equal(21, bp.read_bits(5))
    assert_equal(16, bp.read_pos)
    assert_equal(0xffffffff, bp.read_bits(32))
    assert_equal(48, bp.read_pos)

    # error cases
    assert_raise RangeError, "attempted to read past end of bitpack (last index is 47)" do
      bp.read_bits(1)
    end
    assert_equal(48, bp.read_pos)

    bp.reset_read_pos
    assert_equal(0, bp.read_pos)
    assert_equal(0xff, bp.read_bits(8))
    assert_equal(8, bp.read_pos)
  end

  def test_read_bytes
    test_bytes1 = [ 0x01, 0x02, 0x03 ].pack("C*")
    test_bytes2 = [ 0xff, 0xfe, 0xfd ].pack("C*")
    test_bytes3 = [ 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff ].pack("C*")

    bp = BitPack.new(4);
    bp.append_bytes(test_bytes1);
    bp.append_bytes(test_bytes2);
    bp.append_bits(2, 2);
    bp.append_bytes(test_bytes3);

    assert_equal(0, bp.read_pos)

    assert_equal(test_bytes1, bp.read_bytes(3))
    assert_equal(24, bp.read_pos)

    assert_equal(test_bytes2, bp.read_bytes(3))
    assert_equal(48, bp.read_pos)

    assert_equal(2, bp.read_bits(2))
    assert_equal(50, bp.read_pos)

    assert_equal(test_bytes3, bp.read_bytes(6))
    assert_equal(98, bp.read_pos)

    # error cases
    assert_raise RangeError, "attempted to read past end of bitpack (last index is 97)" do
      bp.read_bytes(1)
    end
    assert_equal(98, bp.read_pos)

    bp.reset_read_pos
    assert_equal(0, bp.read_pos)

    assert_equal(test_bytes1, bp.read_bytes(3))
    assert_equal(24, bp.read_pos)
  end

  def test_to_bytes
    test_bytes1 = [ 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce ].pack("C*")
    test_bytes2 = [ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a ].pack("C*")
    exp_bytes = [ 
        0x1b, 0x91, 0xa2, 0xb3, 0xc0, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa,
        0xce, 0x03, 0x04, 0x11, 0x11, 0x11, 0x11, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    ].pack("C*")
    exp_bytes2 = [ 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce, 0x80 ].pack("C*")
    exp_s = "0001101110010001101000101011001111000000110111101010110110111110111011111111111011101101111110101100111000000011000001000001000100010001000100010001000100000001000000100000001100000100000001010000011000000111000010000000100100001010";

    bp = BitPack.new

    bp.append_bits(3, 5)
    bp.append_bits(3, 3)
    bp.append_bits(0x12345678, 29)
    bp.append_bits(0, 3)
    bp.append_bytes(test_bytes1)
    bp.append_bits(12, 10)
    bp.append_bits(4, 6)
    bp.append_bits(0x11111111, 32)
    bp.append_bytes(test_bytes2)

    assert_equal(exp_bytes, bp.to_bytes)
    assert_equal(exp_bytes.length * 8, bp.size)
    assert_equal(exp_s, bp.to_bin)

    bp = BitPack.new(100);

    bp.append_bytes(test_bytes1)
    bp.append_bits(1, 1);

    assert_equal(exp_bytes2, bp.to_bytes)
    assert_equal(test_bytes1.length * 8 + 1, bp.size)
  end

  def test_from_bytes
    test_bytes1 = [ 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce ].pack("C*")
    test_bytes2 = [ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a ].pack("C*")
    init_bytes = [ 
        0x1b, 0x91, 0xa2, 0xb3, 0xc0, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa,
        0xce, 0x03, 0x04, 0x11, 0x11, 0x11, 0x11, 0x01, 0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09, 0x0a
    ].pack("C*")
    exp_s = "0001101110010001101000101011001111000000110111101010110110111110111011111111111011101101111110101100111000000011000001000001000100010001000100010001000100000001000000100000001100000100000001010000011000000111000010000000100100001010"

    bp = BitPack.from_bytes(init_bytes)

    assert_equal(init_bytes.length * 8, bp.size)
    assert_equal(exp_s, bp.to_bin)

    assert_equal(3, bp.read_bits(5))
    assert_equal(3, bp.read_bits(3))
    assert_equal(0x12345678, bp.read_bits(29))
    assert_equal(0, bp.read_bits(3))
    assert_equal(test_bytes1, bp.read_bytes(test_bytes1.length))
    assert_equal(12, bp.read_bits(10))
    assert_equal(4, bp.read_bits(6))
    assert_equal(0x11111111, bp.read_bits(32))
    assert_equal(test_bytes2, bp.read_bytes(test_bytes2.length))
  end

  def test_assignment_index
    bp = BitPack.new

    bp[0] = 1
    bp[1] = 0
    bp[2] = 1
    bp[3] = 0
    bp[4] = 1
    bp[5] = 0
    bp[6] = 1
    bp[7] = 0

    assert_equal(8, bp.size)
    assert_equal(0b10101010, bp.read_bits(8))
  end

  def test_assignment_range
    bp = BitPack.new

    bp[0..7]   = 0xab
    bp[8...16] = 0xcd

    assert_equal(16, bp.size)
    assert_equal([0xab, 0xcd].pack("C*"), bp.to_bytes)
  end

  def test_assignment_index_length
    bp = BitPack.new

    bp[0, 8] = 0xab
    bp[8, 8] = 0xcd

    assert_equal(16, bp.size)
    assert_equal([0xab, 0xcd].pack("C*"), bp.to_bytes)
  end
end

