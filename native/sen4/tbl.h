#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen4 {
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

    void WriteToStream(WriteStream& s,
                       HyoutaUtils::EndianUtils::Endianness e,
                       HyoutaUtils::TextUtils::GameTextEncoding encoding =
                           HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

private:
    uint16_t GetLength(const std::string& name,
                       ReadStream& stream,
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

struct ItemData {
    uint16_t idx;
    uint16_t character;
    std::string flags;
    std::array<char, 0x96> d1;
    std::string name;
    std::string desc;
    std::vector<char> d2;

    ItemData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MasterQuartzMemo {
    uint16_t mqidx;
    uint16_t stridx;
    std::string str;

    MasterQuartzMemo(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};
} // namespace SenLib::Sen4