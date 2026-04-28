#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "util/file.h"

namespace HyoutaUtils::Csv {
struct CsvRow {
    size_t Index;  // index into Values
    size_t Length; // number of sequential values representing this row
};

struct CsvValue {
    size_t Offset; // position in string
    size_t Length; // number of bytes this value takes
};

struct CsvFile {
    CsvFile();
    CsvFile(const CsvFile& other) = delete;
    CsvFile(CsvFile&& other) = delete;
    CsvFile& operator=(const CsvFile& other) = delete;
    CsvFile& operator=(CsvFile&& other) = delete;
    ~CsvFile();

    // CsvFile takes memory ownership of data.
    // Returns false on IO error or parsing error.
    bool ParseFile(HyoutaUtils::IO::File& file, char separator = ',', bool handleQuotes = true);

    // CsvFile takes memory ownership of data.
    // Returns false on parsing error.
    bool ParseMemory(std::unique_ptr<char[]> buffer,
                     size_t bufferLength,
                     char separator = ',',
                     bool handleQuotes = true);

    // CsvFile does *not* take memory ownership of data, but instead just points at it.
    // Ensure that the memory is deleted *after* CsvFile is destructed.
    // Returns false on parsing error.
    bool ParseExternalMemory(const char* buffer,
                             size_t bufferLength,
                             char separator = ',',
                             bool handleQuotes = true);

    std::span<const CsvRow> GetRows() const {
        return std::span<const CsvRow>(Rows.get(), RowCount);
    }
    std::span<const CsvValue> GetValues() const {
        return std::span<const CsvValue>(Values.get(), ValueCount);
    }
    size_t GetRowCount() const {
        return RowCount;
    }
    std::span<const CsvValue> GetRow(size_t index) const {
        const auto& row = GetRows()[index];
        return GetValues().subspan(row.Index, row.Length);
    }
    const char* GetStoredData() const {
        return DataBuffer.get();
    }
    size_t GetStoredDataLength() const {
        return DataBufferLength;
    }

    // Call this if you want all values in their unescaped form.
    // This will modify the data in-place, so ideally you call this once immediately after parsing.
    // The first variant is for when CsvFile has ownership of the buffer, the second variant is for
    // external buffer data. For the second variant, you *must* pass the same buffer as the one
    // originally passed to ParseExternalMemory(), otherwise the behavior is undefined.
    bool UnescapeAllValues();
    bool UnescapeAllValues(char* data);

private:
    // stores the ini file
    std::unique_ptr<char[]> DataBuffer = nullptr;
    size_t DataBufferLength = 0;

    // these are all references into the DataBuffer
    std::unique_ptr<CsvValue[]> Values = nullptr;
    size_t ValueCount = 0;
    std::unique_ptr<CsvRow[]> Rows = nullptr;
    size_t RowCount = 0;
};

inline std::string_view GetStringView(const char* data, const CsvValue& value) {
    return std::string_view(data + value.Offset, value.Length);
}

// Unescapes a string parsed from a CSV. The given length will be changed to the new length.
// Returns:
// - true if the string is not escaped at all (does not start with a "); string will be unmodified.
// - true if the string was successfully unescaped (["ab"] -> [ab]; ["a""b"] -> [a"b]).
// - false if the string has malformed escape sequences (["a"b"] or ["a"b] or ["a""]).
// Note that a string like [a"] does not count as escaped and will just return true unmodified!
bool UnescapeStringInPlace(char* data, size_t& length);
} // namespace HyoutaUtils::Csv
