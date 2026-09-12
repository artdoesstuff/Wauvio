#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace wauvio {
namespace sf2 {

class Sf2ParseError : public std::runtime_error {
public:
    explicit Sf2ParseError(const std::string& what) : std::runtime_error(what) {}
};

class ByteReader {
public:
    ByteReader(const unsigned char* data, size_t size) : data_(data), size_(size) {}

    size_t position() const noexcept { return pos_; }
    size_t remaining() const noexcept { return pos_ < size_ ? size_ - pos_ : 0; }
    bool eof() const noexcept { return pos_ >= size_; }

    void require(size_t n) const {
        if (pos_ + n > size_ || pos_ + n < pos_)
            throw Sf2ParseError("Unexpected end of SF2 data at offset " + std::to_string(pos_));
    }

    uint8_t u8() { require(1); return data_[pos_++]; }

    uint16_t u16le() {
        require(2);
        uint16_t v = static_cast<uint16_t>(data_[pos_] | (data_[pos_ + 1] << 8));
        pos_ += 2;
        return v;
    }

    uint32_t u32le() {
        require(4);
        uint32_t v = static_cast<uint32_t>(data_[pos_]) | (static_cast<uint32_t>(data_[pos_ + 1]) << 8) |
                     (static_cast<uint32_t>(data_[pos_ + 2]) << 16) | (static_cast<uint32_t>(data_[pos_ + 3]) << 24);
        pos_ += 4;
        return v;
    }

    int16_t s16le() { return static_cast<int16_t>(u16le()); }
    int8_t s8() { return static_cast<int8_t>(u8()); }

    std::string fourcc() {
        require(4);
        std::string s(reinterpret_cast<const char*>(data_ + pos_), 4);
        pos_ += 4;
        return s;
    }

    std::string fixed_string(size_t len) {
        require(len);
        size_t actual = 0;
        while (actual < len && data_[pos_ + actual] != 0) ++actual;
        std::string s(reinterpret_cast<const char*>(data_ + pos_), actual);
        pos_ += len;
        return s;
    }

    void skip(size_t n) { require(n); pos_ += n; }
    void seek(size_t absolute) {
        if (absolute > size_) throw Sf2ParseError("Attempted to seek past end of SF2 data");
        pos_ = absolute;
    }

    const unsigned char* ptr() const noexcept { return data_ + pos_; }
    const unsigned char* base() const noexcept { return data_; }
    size_t size() const noexcept { return size_; }

private:
    const unsigned char* data_;
    size_t size_;
    size_t pos_ = 0;
};

struct RiffChunk {
    std::string id;
    size_t data_offset = 0;
    uint32_t data_size = 0;
};

inline std::vector<unsigned char> read_whole_file(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw Sf2ParseError("Cannot open SoundFont file: " + path);
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    if (sz < 0) { std::fclose(f); throw Sf2ParseError("Cannot determine size of SoundFont file: " + path); }
    std::fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> buf(static_cast<size_t>(sz));
    if (sz > 0) {
        size_t got = std::fread(buf.data(), 1, buf.size(), f);
        if (got != buf.size()) { std::fclose(f); throw Sf2ParseError("Failed to read SoundFont file (truncated): " + path); }
    }
    std::fclose(f);
    return buf;
}

}
}
