#pragma once

#include <cstdint>
#include <cstring>
#include <span>

namespace HyoutaUtils::MemWrite {
// oneshot writes
inline void WriteUInt64(char* target, uint64_t data) {
    std::memcpy(target, &data, 8);
}

inline void WriteUInt32(char* target, uint32_t data) {
    std::memcpy(target, &data, 4);
}

inline void WriteUInt16(char* target, uint16_t data) {
    std::memcpy(target, &data, 2);
}

inline void WriteUInt8(char* target, uint8_t data) {
    std::memcpy(target, &data, 1);
}

inline void WriteArray(char* target, std::span<const char> data) {
    std::memcpy(target, data.data(), data.size());
}

// pointer-advancing writes
inline void WriteAdvUInt64(char*& target, uint64_t data) {
    std::memcpy(target, &data, 8);
    target += 8;
}

inline void WriteAdvUInt32(char*& target, uint32_t data) {
    std::memcpy(target, &data, 4);
    target += 4;
}

inline void WriteAdvUInt16(char*& target, uint16_t data) {
    std::memcpy(target, &data, 2);
    target += 2;
}

inline void WriteAdvUInt8(char*& target, uint8_t data) {
    std::memcpy(target, &data, 1);
    target += 1;
}

inline void WriteAdvArray(char*& target, std::span<const char> data) {
    std::memcpy(target, data.data(), data.size());
    target += data.size();
}
} // namespace HyoutaUtils::MemWrite
