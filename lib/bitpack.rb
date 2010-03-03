
class BitPack
  # Element Assignment. Sets the bit at +index+, or the range of bits indicated
  # by the Range object +range+, or the range specified by +index+ and +length+.
  #
  # :call-seq:
  #   bp[index] = value             -> value
  #   bp[range] = value             -> value
  #   bp[index, length] = value     -> value
  #
  # === Example
  #
  #   >> bp[0] = 1
  #   => 1
  #   >> bp[1] = 0
  #   => 0
  #   >> bp[2] = 1
  #   => 1
  #   >> bp.to_bin
  #   => "101"
  #   >> bp[0..7] = 0xff
  #   => 255
  #   >> bp.to_bin
  #   => "11111111"
  #   >> bp[8, 16] = 0xf0f0
  #   => 61680
  #   >> bp.to_bin
  #   => "111111111111000011110000"
  #
  def []=(a, b, c = nil)
    if c.nil?
      # only two arguments, so it must be one of the following formats:
      #   bp[index] = value
      #   bp[range] = value
      if a.kind_of? Integer
        self.set_bits(b, 1, a)
      elsif a.kind_of? Range
        if a.exclude_end?
          self.set_bits(b, a.end - a.begin, a.begin)
        else
          self.set_bits(b, a.end - a.begin + 1, a.begin)
        end
      else
        raise ArgumentError, "index must be an Integer or Range"
      end
    else
      # this must the following format:
      #   bp[index, length] = value
      self.set_bits(c, b, a)
    end
  end
end

