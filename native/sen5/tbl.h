#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen5 {
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

    TextTableData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemData {
    uint16_t idx;
    uint16_t character;
    std::string flags;
    std::array<char, 141> d1;
    std::string name;
    std::string desc;

    ItemData();
    ItemData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemEData {
    ItemData item;
    std::array<char, 10> e;

    ItemEData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemQData {
    ItemData item;
    std::array<char, 22> q;

    ItemQData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MagicDataEffect {
    uint16_t idx;
    std::array<uint32_t, 3> data;
};

struct MagicData {
    uint16_t idx;
    uint16_t d0;
    std::string flags;
    std::array<char, 22> d1;
    std::array<MagicDataEffect, 5> effects;
    std::array<char, 12> d2;
    std::string animation;
    std::string name;
    std::string desc;

    MagicData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct CookData {
    std::string name;
    std::array<char, 0x22> d1;
    uint16_t item1;
    std::string item1line1;
    std::string item1line2;
    uint16_t item2;
    std::string item2line1;
    std::string item2line2;
    uint16_t item3;
    std::string item3line1;
    std::string item3line2;
    std::array<char, 0x30> d2;

    CookData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};
} // namespace SenLib::Sen5
