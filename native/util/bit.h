#pragma once

#include <cstring>
#include <limits>
#include <type_traits>

// backport of parts of the <bit> header for older C++ versions

namespace HyoutaUtils {
template <typename T, typename U>
T bit_cast(const U& value) noexcept {
    static_assert(sizeof(T) == sizeof(U));
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(std::is_trivially_copyable_v<U>);

    T result;
    std::memcpy(&result, &value, sizeof(T));
    return result;
}

template <typename T>
constexpr int countl_zero(T value) noexcept {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, bool>);

    constexpr int bitcount = sizeof(T) * 8u;
    int count = bitcount;
    int result = 0;
    while (count != 0) {
        --count;
        if (value & (static_cast<T>(1) << count)) {
            break;
        }
        ++result;
    }
    return result;
}

template <typename T>
constexpr int bit_width(T value) noexcept {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, bool>);

    return std::numeric_limits<T>::digits - countl_zero(value);
}

template <typename T>
constexpr T bit_ceil(T value) noexcept {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, bool>);

    if (value <= static_cast<T>(1)) {
        return static_cast<T>(1);
    }
    return static_cast<T>(1) << bit_width<T>(value - 1);
}

template <typename T>
constexpr T bit_floor(T value) noexcept {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);
    static_assert(!std::is_same_v<std::remove_cv_t<T>, bool>);

    if (value == static_cast<T>(0)) {
        return static_cast<T>(0);
    }
    return static_cast<T>(1) << (bit_width<T>(value) - 1);
}
} // namespace HyoutaUtils
