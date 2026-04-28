#include "csv.h"

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "util/file.h"
#include "util/text.h"

namespace HyoutaUtils::Csv {
CsvFile::CsvFile() = default;
CsvFile::~CsvFile() = default;

bool CsvFile::ParseFile(HyoutaUtils::IO::File& file, char separator, bool handleQuotes) {
    if (!file.IsOpen()) {
        return false;
    }
    const auto length = file.GetLength();
    if (!length) {
        return false;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (file.Read(buffer.get(), *length) != *length) {
        return false;
    }

    return ParseMemory(std::move(buffer), *length, separator, handleQuotes);
}

static bool ParseMemoryInternal(const char* buffer,
                                size_t bufferLength,
                                char separator,
                                bool handleQuotes,
                                std::vector<CsvValue>& values,
                                std::vector<CsvRow>& rows) {
    size_t pos = 0;
    size_t rowStart = 0;
    size_t valueStart = 0;
    size_t rowIndex = 0;

    // skip UTF8 BOM if it's there
    if (bufferLength >= 3 && buffer[0] == '\xef' && buffer[1] == '\xbb' && buffer[2] == '\xbf') {
        pos = 3;
    }

    while (true) {
        if (pos == bufferLength) {
            // end of buffer
            return true;
        }

        const char c = buffer[pos];
        if (c == separator) {
            // empty value
            values.emplace_back(CsvValue{valueStart, pos - valueStart});
            ++pos;
            valueStart = pos;
            continue;
        }
        if (handleQuotes && c == '"') {
            // quoted value, scan forwards until the next quote
            ++pos;
            while (true) {
                if (pos == bufferLength) {
                    // reached the end of the buffer mid quoted string, definitely invalid
                    return false;
                }
                const char c2 = buffer[pos];
                if (c2 == '"') {
                    ++pos;
                    if (pos == bufferLength) {
                        // end of quoted value + end of buffer
                        values.emplace_back(CsvValue{valueStart, pos - valueStart});
                        rows.emplace_back(
                            CsvRow{.Index = rowIndex, .Length = values.size() - rowIndex});
                        return true;
                    }
                    const char c3 = buffer[pos];
                    if (c3 == separator) {
                        // end of quoted value
                        values.emplace_back(CsvValue{valueStart, pos - valueStart});
                        ++pos;
                        valueStart = pos;
                        break;
                    } else if (c3 == '"') {
                        // the second quote escapes the first, skip it
                        ++pos;
                    } else if (c3 == '\n' || c3 == '\r') {
                        // end of quoted value + next row
                        values.emplace_back(CsvValue{valueStart, pos - valueStart});
                        rows.emplace_back(
                            CsvRow{.Index = rowIndex, .Length = values.size() - rowIndex});
                        rowIndex = values.size();
                        ++pos;
                        rowStart = pos;
                        valueStart = pos;
                        break;
                    } else {
                        // invalid quote, malformed csv
                        return false;
                    }
                } else {
                    ++pos;
                }
            }
            continue;
        }

        // regular value
        while (true) {
            if (pos == bufferLength) {
                // end of buffer is end of value, except if the entire line is empty
                if (pos != rowStart) {
                    values.emplace_back(CsvValue{valueStart, pos - valueStart});
                    rows.emplace_back(
                        CsvRow{.Index = rowIndex, .Length = values.size() - rowIndex});
                    rowIndex = values.size();
                }
                rowStart = pos;
                valueStart = pos;
                break;
            }
            const char c2 = buffer[pos];
            if (c2 == separator) {
                // end of value
                values.emplace_back(CsvValue{valueStart, pos - valueStart});
                ++pos;
                valueStart = pos;
                break;
            }
            if (handleQuotes && c2 == '"') {
                // mid-value quote, this string should have been escaped!
                return false;
            }
            if (c2 == '\n' || c2 == '\r') {
                // end of line is also end of value, except if the entire line is empty
                if (pos != rowStart) {
                    values.emplace_back(CsvValue{valueStart, pos - valueStart});
                    rows.emplace_back(
                        CsvRow{.Index = rowIndex, .Length = values.size() - rowIndex});
                    rowIndex = values.size();
                }
                ++pos;
                rowStart = pos;
                valueStart = pos;
                break;
            }
            ++pos;
        }
    }
}

bool CsvFile::ParseExternalMemory(const char* buffer,
                                  size_t bufferLength,
                                  char separator,
                                  bool handleQuotes) {
    std::vector<CsvValue> values;
    std::vector<CsvRow> rows;
    if (!ParseMemoryInternal(buffer, bufferLength, separator, handleQuotes, values, rows)) {
        return false;
    }

    Values = std::make_unique_for_overwrite<CsvValue[]>(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        Values[i] = std::move(values[i]);
    }
    ValueCount = values.size();
    Rows = std::make_unique_for_overwrite<CsvRow[]>(rows.size());
    for (size_t i = 0; i < rows.size(); ++i) {
        Rows[i] = std::move(rows[i]);
    }
    RowCount = rows.size();
    return true;
}

bool CsvFile::ParseMemory(std::unique_ptr<char[]> buffer,
                          size_t bufferLength,
                          char separator,
                          bool handleQuotes) {
    if (!ParseExternalMemory(buffer.get(), bufferLength, separator, handleQuotes)) {
        return false;
    }
    DataBuffer = std::move(buffer);
    DataBufferLength = bufferLength;
    return true;
}

bool UnescapeStringInPlace(char* data, size_t& length) {
    if (length == 0 || data[0] != '"') {
        return true;
    }
    size_t pos = 1;
    size_t sourceLength = length;
    size_t targetLength = 0;
    while (true) {
        if (pos == sourceLength) {
            return false;
        }
        const char c = data[pos];
        if (c == '"') {
            ++pos;
            if (pos == sourceLength) {
                length = targetLength;
                return true;
            }
            if (data[pos] != '"') {
                return false;
            }
        }
        data[targetLength] = c;
        ++targetLength;
        ++pos;
    }
}

bool CsvFile::UnescapeAllValues() {
    return UnescapeAllValues(DataBuffer.get());
}
bool CsvFile::UnescapeAllValues(char* data) {
    const size_t count = ValueCount;
    CsvValue* value = Values.get();
    for (size_t i = 0; i < count; ++i) {
        if (!UnescapeStringInPlace(data + value->Offset, value->Length)) {
            return false;
        }
        ++value;
    }
    return true;
}
} // namespace HyoutaUtils::Csv
