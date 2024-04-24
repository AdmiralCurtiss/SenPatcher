#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace HyoutaUtils::MemRead {
// oneshot reads
inline uint64_t ReadUInt64(const char* source) {
    uint64_t data;
    std::memcpy(&data, source, 8);
    return data;
}

inline uint32_t ReadUInt32(const char* source) {
    uint32_t data;
    std::memcpy(&data, source, 4);
    return data;
}

inline uint16_t ReadUInt16(const char* source) {
    uint16_t data;
    std::memcpy(&data, source, 2);
    return data;
}

inline uint8_t ReadUInt8(const char* source) {
    uint8_t data;
    std::memcpy(&data, source, 1);
    return data;
}

template<size_t Size>
inline std::array<char, Size> ReadArray(const char* source) {
    std::array<char, Size> data;
    std::memcpy(data.data(), source, Size);
    return data;
}

// pointer-advancing reads
inline uint64_t ReadAdvUInt64(const char*& source) {
    uint64_t data;
    std::memcpy(&data, source, 8);
    source += 8;
    return data;
}

inline uint32_t ReadAdvUInt32(const char*& source) {
    uint32_t data;
    std::memcpy(&data, source, 4);
    source += 4;
    return data;
}

inline uint16_t ReadAdvUInt16(const char*& source) {
    uint16_t data;
    std::memcpy(&data, source, 2);
    source += 2;
    return data;
}

inline uint8_t ReadAdvUInt8(const char*& source) {
    uint8_t data;
    std::memcpy(&data, source, 1);
    source += 1;
    return data;
}

template<size_t Size>
inline std::array<char, Size> ReadAdvArray(const char*& source) {
    std::array<char, Size> data;
    std::memcpy(data.data(), source, Size);
    source += Size;
    return data;
}
} // namespace HyoutaUtils::MemRead
