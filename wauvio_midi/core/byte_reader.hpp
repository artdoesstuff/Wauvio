#pragma once

#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace wauvio {
namespace midi {

class MidiParseError : public std::runtime_error {
public:
    explicit MidiParseError(const std::string& what) : std::runtime_error(what) {}
};

class ByteReader {
public:
    ByteReader(const unsigned char* data, size_t size) : data_(data), size_(size) {}

    bool eof() const noexcept { return pos_ >= size_; }
    size_t remaining() const noexcept { return pos_ < size_ ? size_ - pos_ : 0; }
    size_t position() const noexcept { return pos_; }

    void require(size_t n) const {
        if (pos_ + n > size_ || pos_ + n < pos_)
            throw MidiParseError("Unexpected end of MIDI data at byte offset " + std::to_string(pos_));
    }

    uint8_t u8() {
        require(1);
        return data_[pos_++];
    }

    uint16_t u16be() {
        require(2);
        uint16_t v = static_cast<uint16_t>((data_[pos_] << 8) | data_[pos_ + 1]);
        pos_ += 2;
        return v;
    }

    uint32_t u32be() {
        require(4);
        uint32_t v = (static_cast<uint32_t>(data_[pos_]) << 24) |
                     (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                     (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                     static_cast<uint32_t>(data_[pos_ + 3]);
        pos_ += 4;
        return v;
    }

    void skip(size_t n) {
        require(n);
        pos_ += n;
    }

    void seek(size_t absolute_pos) {
        if (absolute_pos > size_)
            throw MidiParseError("Attempted to seek past end of MIDI data");
        pos_ = absolute_pos;
    }

    uint32_t read_vlq() {
        uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t b = u8();
            value = (value << 7) | static_cast<uint32_t>(b & 0x7F);
            if (!(b & 0x80)) return value;
        }
        throw MidiParseError("Invalid variable-length quantity (exceeds 4 bytes) at offset " +
                              std::to_string(pos_));
    }

    const unsigned char* ptr() const noexcept { return data_ + pos_; }

private:
    const unsigned char* data_;
    size_t size_;
    size_t pos_ = 0;
};

}
}
