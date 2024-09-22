#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::TX {
struct TblDefinition {
    std::string Name;
    uint32_t NumberOfEntries;
};

struct TblEntry {
    std::string Name;
    std::vector<char> Data;
};

struct Tbl {
    std::vector<TblDefinition> Definitions;
    std::vector<TblEntry> Entries;

    Tbl(const char* buffer,
        size_t length,
        HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian,
        HyoutaUtils::TextUtils::GameTextEncoding encoding =
            HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    void RecalcNumberOfEntries();

    void WriteToStream(HyoutaUtils::Stream::WriteStream& s,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding =
                           HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

private:
    uint16_t GetLength(const std::string& name,
                       HyoutaUtils::Stream::ReadStream& stream,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding) const;
};

struct TextTableData {
    uint16_t idx;
    std::string str;
    std::vector<char> d;

    TextTableData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};
} // namespace SenLib::TX
