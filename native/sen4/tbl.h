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

struct ItemData {
    uint16_t idx;
    uint16_t character;
    std::string flags;
    std::array<char, 2> d1;
    uint8_t category;
    std::array<char, 0x61> d2;
    int32_t STR;
    int32_t DEF;
    int32_t ATS;
    int32_t ADF;
    int32_t ACC;
    int32_t EVA;
    int32_t SPD;
    int32_t MOV;
    int32_t HP;
    int32_t EP;
    std::array<char, 10> d3;
    std::string name;
    std::string desc;
    std::array<char, 8> d4;

    ItemData();
    ItemData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct ItemQData {
    ItemData item;
    std::array<uint16_t, 6> arts;

    ItemQData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MasterQuartzMemo {
    uint16_t mqidx;
    uint16_t stridx;
    std::string str;

    MasterQuartzMemo(const char* data, size_t dataLength);

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
    std::array<char, 11> d2;
    std::string animation;
    std::string name;
    std::string desc;

    MagicData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct CookDataItem {
    uint16_t id;
    std::array<std::string, 2> lines;
};

struct CookData {
    std::string name;
    std::array<char, 0x22> d1;
    std::array<CookDataItem, 4> items;
    std::array<char, 0x30> d2;

    CookData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};
} // namespace SenLib::Sen4
