#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace HyoutaUtils::Vector {
// writes 'data' to [&vector[offset], &vector[offset+data.size()]), overwriting any existing data
// and expanding the vector if necessary
template<typename T>
void WriteAt(std::vector<T>& vector, size_t offset, std::span<const T> data) {
    while (vector.size() < offset) {
        vector.emplace_back();
    }

    for (size_t i = 0; i < data.size(); ++i) {
        const size_t pos = offset + i;
        if (pos < vector.size()) {
            vector[pos] = data[i];
        } else {
            vector.push_back(data[i]);
        }
    }
}

template<typename T>
void WriteAtEnd(std::vector<T>& vector, std::span<const T> data) {
    vector.insert(vector.end(), data.begin(), data.end());
}

inline std::span<const char> GetSpan(std::span<const char> data, size_t offset, size_t length) {
    return data.subspan(offset, length);
}

inline std::vector<char> GetVector(std::span<const char> data) {
    return std::vector<char>(data.begin(), data.end());
}

inline std::vector<char> GetVector(std::span<const char> data, size_t offset, size_t length) {
    return GetVector(GetSpan(data, offset, length));
}

// Moves data from [sourceLocation, sourceLocation + length) to targetLocation, shifting all
// data in between to the side. Effectively cutting out some data and pasting it elsewhere.
bool ShiftData(std::span<char> data, size_t sourceLocation, size_t targetLocation, size_t length);
} // namespace HyoutaUtils::Vector
