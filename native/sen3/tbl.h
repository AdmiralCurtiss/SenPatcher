#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen3 {
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
    uint8_t category;
    std::array<char, 13> d2;
    std::array<uint16_t, 7> effect1;
    std::array<uint16_t, 7> effect2;
    std::array<uint16_t, 7> effect3;
    std::array<uint16_t, 7> effect4;
    std::array<uint16_t, 7> effect5;
    std::array<uint16_t, 7> effect6;
    int16_t STR;
    int16_t DEF;
    int16_t ATS;
    int16_t ADF;
    int16_t ACC;
    int16_t EVA;
    int16_t SPD;
    int16_t MOV;
    int16_t HP;
    int16_t EP;
    std::array<char, 9> d3;
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

struct ItemHelpData {
    uint16_t idx;
    std::string str;
    uint16_t d1;
    std::array<char, 7> d2;

    ItemHelpData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct CompHelpData {
    uint16_t idx;
    std::string str;
    std::vector<char> d;

    CompHelpData(const char* data, size_t dataLength);

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
    std::array<char, 10> d2;
    std::string animation;
    std::string name;
    std::string desc;

    MagicData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MonsterData {
    std::string file1;
    std::string file2;
    std::string file3;
    std::array<char, 0xc5> d0;
    std::string flags;
    std::string name;
    std::string desc;

    MonsterData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct MasterQuartzMemo {
    uint16_t mqidx;
    uint16_t stridx;
    std::string str;

    MasterQuartzMemo(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct NameData {
    uint16_t idx;
    std::string name;
    std::string str2;
    std::string str3;
    std::string str4;
    std::string str5;
    std::string str6;
    std::vector<char> d2;

    NameData(const char* data, size_t dataLength);

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

struct ChapterData {
    uint16_t idx;
    std::array<char, 2> d1;
    std::string name;

    ChapterData(const char* data, size_t dataLength);

    std::vector<char> ToBinary() const;
};

struct VoiceTimingData {
    uint16_t Index;
    std::vector<char> Unknown;

    VoiceTimingData(const char* data,
                    size_t dataLength,
                    HyoutaUtils::EndianUtils::Endianness e,
                    HyoutaUtils::TextUtils::GameTextEncoding encoding =
                        HyoutaUtils::TextUtils::GameTextEncoding::UTF8);

    std::vector<char> ToBinary(HyoutaUtils::EndianUtils::Endianness e,
                               HyoutaUtils::TextUtils::GameTextEncoding encoding =
                                   HyoutaUtils::TextUtils::GameTextEncoding::UTF8) const;
};
} // namespace SenLib::Sen3
