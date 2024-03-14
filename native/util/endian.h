#pragma once

#include <bit>
#include <cstdint>

namespace HyoutaUtils::EndianUtils {
enum class Endianness {
    LittleEndian,
    BigEndian,
};

inline char SwapEndian4Bits(char x) {
    return (char)(((x & 0x0F) << 4) | ((x & 0xF0) >> 4));
}

inline uint16_t SwapEndian(uint16_t x) {
    return (uint16_t)((x << 8) | (x >> 8));
}

inline int16_t SwapEndian(int16_t x) {
    return (int16_t)SwapEndian((uint16_t)x);
}

inline uint32_t SwapEndian24(uint32_t x) {
    return ((x & 0x000000FF) << 16) | ((x & 0x0000FF00)) | ((x & 0x00FF0000) >> 16);
}

inline int32_t SwapEndian24(int32_t x) {
    return (int32_t)SwapEndian24((uint32_t)x);
}

inline uint32_t SwapEndian(uint32_t x) {
    return (x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24);
}

inline int32_t SwapEndian(int32_t x) {
    return (int32_t)SwapEndian((uint32_t)x);
}

inline uint64_t SwapEndian(uint64_t x) {
    return x = (x << 56) | ((x << 40) & 0x00FF000000000000) | ((x << 24) & 0x0000FF0000000000)
               | ((x << 8) & 0x000000FF00000000) | ((x >> 8) & 0x00000000FF000000)
               | ((x >> 24) & 0x0000000000FF0000) | ((x >> 40) & 0x000000000000FF00) | (x >> 56);
}

inline int64_t SwapEndian(int64_t x) {
    return (int64_t)SwapEndian((uint64_t)x);
}

inline uint16_t ToEndian(uint16_t x, Endianness endian) {
    switch (endian) {
        case Endianness::LittleEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return x;
            } else {
                return SwapEndian(x);
            }
        case Endianness::BigEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return SwapEndian(x);
            } else {
                return x;
            }
        default: throw "Invalid Endianness";
    }
}

inline int16_t ToEndian(int16_t x, Endianness endian) {
    return (int16_t)ToEndian((uint16_t)x, endian);
}

inline uint32_t ToEndian(uint32_t x, Endianness endian) {
    switch (endian) {
        case Endianness::LittleEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return x;
            } else {
                return SwapEndian(x);
            }
        case Endianness::BigEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return SwapEndian(x);
            } else {
                return x;
            }
        default: throw "Invalid Endianness";
    }
}

inline int32_t ToEndian(int32_t x, Endianness endian) {
    return (int32_t)ToEndian((uint32_t)x, endian);
}

inline uint64_t ToEndian(uint64_t x, Endianness endian) {
    switch (endian) {
        case Endianness::LittleEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return x;
            } else {
                return SwapEndian(x);
            }
        case Endianness::BigEndian:
            if constexpr (std::endian::native == std::endian::little) {
                return SwapEndian(x);
            } else {
                return x;
            }
        default: throw "Invalid Endianness";
    }
}

inline int64_t ToEndian(int64_t x, Endianness endian) {
    return (int64_t)ToEndian((uint64_t)x, endian);
}

// honestly I'm not sure if it makes sense to have different To and From functions
// since all cases I can think of result in the same thing, but better be safe than sorry,
// and it also gives some information if we're reading in or writing out data
inline uint16_t FromEndian(uint16_t x, Endianness endian) {
    return ToEndian(x, endian);
}

inline int16_t FromEndian(int16_t x, Endianness endian) {
    return ToEndian(x, endian);
}

inline uint32_t FromEndian(uint32_t x, Endianness endian) {
    return ToEndian(x, endian);
}

inline int32_t FromEndian(int32_t x, Endianness endian) {
    return ToEndian(x, endian);
}

inline uint64_t FromEndian(uint64_t x, Endianness endian) {
    return ToEndian(x, endian);
}

inline int64_t FromEndian(int64_t x, Endianness endian) {
    return ToEndian(x, endian);
}
} // namespace HyoutaUtils::EndianUtils
