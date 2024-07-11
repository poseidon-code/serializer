#pragma once

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


enum Endianness {
    BO_LITTLE_ENDIAN,
    BO_BIG_ENDIAN
};



namespace serializer {
    constexpr bool _is_system_little_endian() {
        union {int16_t value; uint8_t bytes[sizeof(value)];} x;
        x.value = 1;
        return x.bytes[0] == 1;
    };


    void _serialize(uint8_t* stream, const uint8_t* bytes, uint8_t byte_size, size_t index_start, Endianness endianness) {
        if (endianness == Endianness::BO_LITTLE_ENDIAN && _is_system_little_endian()) {
            std::copy(bytes, bytes + byte_size, stream + index_start);
        } else {
            std::reverse_copy(bytes, bytes + byte_size, stream + index_start);
        }
    }


    void _deserialize(const uint8_t* stream, uint8_t* bytes, uint8_t byte_size, size_t index_start, Endianness endianness) {
        if (endianness == Endianness::BO_LITTLE_ENDIAN && _is_system_little_endian()) {
            std::copy(stream + index_start, stream + index_start + byte_size, bytes);
        } else {
            std::reverse_copy(stream + index_start, stream + index_start + byte_size, bytes);
        }
    }



    template <typename T, Endianness endianness = Endianness::BO_BIG_ENDIAN>
    class byte_t {
    private:
        const uint8_t byte_size = sizeof(T);

        union {
            T value;
            uint8_t bytes[sizeof(T)];
        } t;

    public:
        void serialize(uint8_t* stream, T value, size_t index_start = 0) {
            this->t.value = value;
            _serialize(stream, this->t.bytes, this->byte_size, index_start, endianness);
        }

        void serialize(std::vector<uint8_t>& stream, T value, size_t index_start = 0) {
            this->serialize(stream.data(), value, index_start);
        }

        std::vector<uint8_t> serialize(T value) {
            std::vector<uint8_t> buffer(this->byte_size, 0x00);
            this->serialize(buffer.data(), value, 0);
            return buffer;
        }

        T deserialize(const uint8_t* stream, size_t index_start = 0) {
            this->t.value = 0;
            _deserialize(stream, this->t.bytes, this->byte_size, index_start, endianness);
            return t.value;
        }

        T deserialize(const std::vector<uint8_t>& stream, size_t index_start = 0) {
            return this->deserialize(stream.data(), index_start);
        }

        friend std::ostream& operator<<(std::ostream& os, const byte_t& byte) {
            os << "0x ";
            os << std::hex << std::uppercase << std::setfill('0');
            for (uint8_t i = 0; i < byte.byte_size; ++i) {
                if (endianness == Endianness::BO_LITTLE_ENDIAN && _is_system_little_endian()) {
                    os << std::setw(2) << static_cast<uint16_t>(byte.t.bytes[i]);
                } else {
                    os << std::setw(2) << static_cast<uint16_t>(byte.t.bytes[byte.byte_size - 1 - i]);
                }

                if (i != byte.byte_size - 1) os << " ";
            }
            os << std::dec << std::nouppercase << std::setfill(' ');
            return os;
        }
    };



    class stream {
    private:
        std::vector<uint8_t> buffer;
        size_t index;

    public:
        stream() = delete;

        stream(size_t length)
            : buffer(length, 0x00), index(0)
        {};

        stream(const stream& other)
            : buffer(other.buffer), index(other.index)
        {}

        stream(stream&& other) noexcept
            : buffer(std::move(other.buffer)), index(other.index)
        { other.index = 0; }

        stream& operator=(const stream& other) {
            if (this != &other) {
                this->buffer = other.buffer;
                this->index = other.index;
            }
            return *this;
        }

        stream& operator=(stream&& other) noexcept {
            if (this != &other) {
                this->buffer = std::move(other.buffer);
                this->index = other.index;
                other.index = 0;
            }
            return *this;
        }

        ~stream() = default;

        std::vector<uint8_t> const get() const {
            return this->buffer;
        }

        stream& operator<<(const std::vector<uint8_t>& buffer) {
            std::copy(buffer.begin(), buffer.end(), this->buffer.begin() + this->index);
            this->index += buffer.size();
            return *this;
        }

        void put(const uint8_t* buffer, size_t length, size_t index_start = 0) {
            std::copy(buffer, buffer + length, this->buffer.begin() + index_start);
            this->index = this->index + (index_start - this->index) + length;
        }

        void put(const std::vector<uint8_t>& buffer, size_t index_start = 0) {
            this->put(buffer.data(), buffer.size(), index_start);
        }
    };



    void print(const uint8_t* stream, size_t length, std::string delimeter = " ") {
        std::cout << std::hex << std::uppercase << std::setfill('0');
        for (size_t i = 0; i < length; ++i)
            std::cout << std::setw(2)  << static_cast<uint>(stream[i]) << (i == length - 1 ? "" : delimeter);
        std::cout << std::dec << std::nouppercase << std::setfill(' ');
    }

    void print(const std::vector<uint8_t>& stream, std::string delimeter = " ") {
        print(stream.data(), stream.size(), delimeter);
    }

    void print(const serializer::stream& stream, std::string delimeter = " ") {
        print(stream.get().data(), stream.get().size(), delimeter);
    }

    std::string sprint(const uint8_t* stream, size_t length, std::string delimeter = " ") {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0');
        for (size_t i = 0; i < length; ++i)
            oss << std::setw(2) << static_cast<uint>(stream[i]) << (i == length - 1 ? "" : delimeter);
        std::cout << std::dec << std::nouppercase << std::setfill(' ');
        return oss.str();
    }

    std::string sprint(const std::vector<uint8_t>& stream, std::string delimeter = " ") {
        return sprint(stream.data(), stream.size(), delimeter);
    }

    std::string sprint(const serializer::stream& stream, std::string delimeter = " ") {
        return sprint(stream.get().data(), stream.get().size(), delimeter);
    }
}