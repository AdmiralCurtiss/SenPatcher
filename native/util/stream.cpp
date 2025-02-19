#include "stream.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "text.h"

namespace HyoutaUtils::Stream {
ReadStream::~ReadStream() = default;

int ReadStream::ReadByte() {
    char result;
    if (Read(&result, 1) == 1) {
        return static_cast<uint8_t>(result);
    }
    return -1;
}

uint64_t ReadStream::ReadUInt64(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 8> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint64";
    }

    const uint64_t b1 = static_cast<uint8_t>(b[0]);
    const uint64_t b2 = static_cast<uint8_t>(b[1]);
    const uint64_t b3 = static_cast<uint8_t>(b[2]);
    const uint64_t b4 = static_cast<uint8_t>(b[3]);
    const uint64_t b5 = static_cast<uint8_t>(b[4]);
    const uint64_t b6 = static_cast<uint8_t>(b[5]);
    const uint64_t b7 = static_cast<uint8_t>(b[6]);
    const uint64_t b8 = static_cast<uint8_t>(b[7]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint64_t)(b8 << 56 | b7 << 48 | b6 << 40 | b5 << 32 | b4 << 24 | b3 << 16
                              | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint64_t)(b1 << 56 | b2 << 48 | b3 << 40 | b4 << 32 | b5 << 24 | b6 << 16
                              | b7 << 8 | b8);
        default: throw "unknown endianness";
    }
}

uint64_t ReadStream::ReadUInt56(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 7> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint56";
    }

    const uint64_t b1 = static_cast<uint8_t>(b[0]);
    const uint64_t b2 = static_cast<uint8_t>(b[1]);
    const uint64_t b3 = static_cast<uint8_t>(b[2]);
    const uint64_t b4 = static_cast<uint8_t>(b[3]);
    const uint64_t b5 = static_cast<uint8_t>(b[4]);
    const uint64_t b6 = static_cast<uint8_t>(b[5]);
    const uint64_t b7 = static_cast<uint8_t>(b[6]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint64_t)(b7 << 48 | b6 << 40 | b5 << 32 | b4 << 24 | b3 << 16 | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint64_t)(b1 << 48 | b2 << 40 | b3 << 32 | b4 << 24 | b5 << 16 | b6 << 8 | b7);
        default: throw "unknown endianness";
    }
}

uint64_t ReadStream::ReadUInt48(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 6> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint48";
    }

    const uint64_t b1 = static_cast<uint8_t>(b[0]);
    const uint64_t b2 = static_cast<uint8_t>(b[1]);
    const uint64_t b3 = static_cast<uint8_t>(b[2]);
    const uint64_t b4 = static_cast<uint8_t>(b[3]);
    const uint64_t b5 = static_cast<uint8_t>(b[4]);
    const uint64_t b6 = static_cast<uint8_t>(b[5]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint64_t)(b6 << 40 | b5 << 32 | b4 << 24 | b3 << 16 | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint64_t)(b1 << 40 | b2 << 32 | b3 << 24 | b4 << 16 | b5 << 8 | b6);
        default: throw "unknown endianness";
    }
}

uint64_t ReadStream::ReadUInt40(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 5> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint40";
    }

    const uint64_t b1 = static_cast<uint8_t>(b[0]);
    const uint64_t b2 = static_cast<uint8_t>(b[1]);
    const uint64_t b3 = static_cast<uint8_t>(b[2]);
    const uint64_t b4 = static_cast<uint8_t>(b[3]);
    const uint64_t b5 = static_cast<uint8_t>(b[4]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint64_t)(b5 << 32 | b4 << 24 | b3 << 16 | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint64_t)(b1 << 32 | b2 << 24 | b3 << 16 | b4 << 8 | b5);
        default: throw "unknown endianness";
    }
}

uint32_t ReadStream::ReadUInt32(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 4> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint32";
    }

    const uint32_t b1 = static_cast<uint8_t>(b[0]);
    const uint32_t b2 = static_cast<uint8_t>(b[1]);
    const uint32_t b3 = static_cast<uint8_t>(b[2]);
    const uint32_t b4 = static_cast<uint8_t>(b[3]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint32_t)(b4 << 24 | b3 << 16 | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint32_t)(b1 << 24 | b2 << 16 | b3 << 8 | b4);
        default: throw "unknown endianness";
    }
}

uint32_t ReadStream::ReadUInt24(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 3> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint24";
    }

    const uint32_t b1 = static_cast<uint8_t>(b[0]);
    const uint32_t b2 = static_cast<uint8_t>(b[1]);
    const uint32_t b3 = static_cast<uint8_t>(b[2]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            return (uint32_t)(b3 << 16 | b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            return (uint32_t)(b1 << 16 | b2 << 8 | b3);
        default: throw "unknown endianness";
    }
}

uint16_t ReadStream::ReadUInt16(HyoutaUtils::EndianUtils::Endianness endian) {
    std::array<char, 2> b;
    if (Read(b.data(), b.size()) != b.size()) {
        throw "cannot read uint16";
    }

    const uint32_t b1 = static_cast<uint8_t>(b[0]);
    const uint32_t b2 = static_cast<uint8_t>(b[1]);
    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian: return (uint16_t)(b2 << 8 | b1);
        case HyoutaUtils::EndianUtils::Endianness::BigEndian: return (uint16_t)(b1 << 8 | b2);
        default: throw "unknown endianness";
    }
}

uint8_t ReadStream::ReadUInt8() {
    const int r = ReadByte();
    if (r < 0 || r > 255) {
        throw "cannot read uint8";
    }
    return static_cast<uint8_t>(r);
}

std::vector<char> ReadStream::ReadVector(size_t length) {
    std::vector<char> rv;
    rv.resize(length);
    if (Read(rv.data(), rv.size()) != rv.size()) {
        throw "cannot read data";
    }
    return rv;
}

std::string ReadStream::ReadAsciiNulltermFromLocationAndReset(size_t location) {
    const auto pos = GetPosition();
    SetPosition(location);
    std::string str = ReadAsciiNullterm();
    SetPosition(pos);
    return str;
}

std::string ReadStream::ReadAsciiNullterm() {
    std::string sb;
    int b = ReadByte();
    while (b != 0 && b != -1) {
        sb.push_back((char)(b));
        b = ReadByte();
    }
    return sb;
}

std::string ReadStream::ReadUTF8Nullterm() {
    std::string sb;
    int b = ReadByte();
    while (b != 0 && b != -1) {
        sb.push_back((char)(b));
        b = ReadByte();
    }
    return sb;
}

std::string ReadStream::ReadUTF16Nullterm() {
    std::u16string sb;
    int b0 = ReadByte();
    int b1 = ReadByte();
    while (!(b0 == 0 && b1 == 0) && b1 != -1) {
        char16_t c16 = (char16_t)((b1 << 8) | b0);
        sb.push_back(c16);
        b0 = ReadByte();
        b1 = ReadByte();
    }

    auto utf8 = HyoutaUtils::TextUtils::Utf16ToUtf8(sb.data(), sb.size());
    if (!utf8) {
        throw "Invalid UTF-16 string";
    }
    return *utf8;
}

std::string ReadStream::ReadShiftJisNullterm() {
    std::string sb;
    int b = ReadByte();
    while (b != 0 && b != -1) {
        if ((b >= 0 && b <= 0x80) || (b >= 0xA0 && b <= 0xDF)) {
            // is a single byte
            sb.push_back(static_cast<char>(b));
        } else {
            // is two bytes
            sb.push_back(static_cast<char>(b));
            b = ReadByte();
            if (b == -1) {
                throw "Invalid Shift-JIS string";
            }
            sb.push_back(static_cast<char>(b));
        }
        b = ReadByte();
    }

    auto utf8 = HyoutaUtils::TextUtils::ShiftJisToUtf8(sb.data(), sb.size());
    if (!utf8) {
        throw "Invalid Shift-JIS string";
    }
    return *utf8;
}

std::string ReadStream::ReadNulltermString(HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    switch (encoding) {
        case HyoutaUtils::TextUtils::GameTextEncoding::ASCII: return ReadAsciiNullterm();
        case HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS: return ReadShiftJisNullterm();
        case HyoutaUtils::TextUtils::GameTextEncoding::UTF8: return ReadUTF8Nullterm();
        case HyoutaUtils::TextUtils::GameTextEncoding::UTF16: return ReadUTF16Nullterm();
    }
    throw "Reading nullterminated string not implemented for encoding";
}

WriteStream::~WriteStream() = default;

void WriteStream::WriteByte(uint8_t byte) {
    char c = static_cast<char>(byte);
    if (Write(&c, 1) != 1) {
        throw "failed to write data";
    }
}

void WriteStream::WriteUInt64(uint64_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((num >> 24) & 0xFF);
    uint8_t b5 = (uint8_t)((num >> 32) & 0xFF);
    uint8_t b6 = (uint8_t)((num >> 40) & 0xFF);
    uint8_t b7 = (uint8_t)((num >> 48) & 0xFF);
    uint8_t b8 = (uint8_t)((num >> 56) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            WriteByte(b4);
            WriteByte(b5);
            WriteByte(b6);
            WriteByte(b7);
            WriteByte(b8);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b8);
            WriteByte(b7);
            WriteByte(b6);
            WriteByte(b5);
            WriteByte(b4);
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt56(uint64_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((num >> 24) & 0xFF);
    uint8_t b5 = (uint8_t)((num >> 32) & 0xFF);
    uint8_t b6 = (uint8_t)((num >> 40) & 0xFF);
    uint8_t b7 = (uint8_t)((num >> 48) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            WriteByte(b4);
            WriteByte(b5);
            WriteByte(b6);
            WriteByte(b7);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b7);
            WriteByte(b6);
            WriteByte(b5);
            WriteByte(b4);
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt48(uint64_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((num >> 24) & 0xFF);
    uint8_t b5 = (uint8_t)((num >> 32) & 0xFF);
    uint8_t b6 = (uint8_t)((num >> 40) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            WriteByte(b4);
            WriteByte(b5);
            WriteByte(b6);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b6);
            WriteByte(b5);
            WriteByte(b4);
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt40(uint64_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((num >> 24) & 0xFF);
    uint8_t b5 = (uint8_t)((num >> 32) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            WriteByte(b4);
            WriteByte(b5);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b5);
            WriteByte(b4);
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt32(uint32_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);
    uint8_t b4 = (uint8_t)((num >> 24) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            WriteByte(b4);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b4);
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt24(uint32_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);
    uint8_t b3 = (uint8_t)((num >> 16) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            WriteByte(b3);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b3);
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt16(uint16_t num, HyoutaUtils::EndianUtils::Endianness endian) {
    uint8_t b1 = (uint8_t)(num & 0xFF);
    uint8_t b2 = (uint8_t)((num >> 8) & 0xFF);

    switch (endian) {
        case HyoutaUtils::EndianUtils::Endianness::LittleEndian:
            WriteByte(b1);
            WriteByte(b2);
            return;
        case HyoutaUtils::EndianUtils::Endianness::BigEndian:
            WriteByte(b2);
            WriteByte(b1);
            return;
        default: throw "unknown endianness";
    }
}

void WriteStream::WriteUInt8(uint8_t num) {
    WriteByte(num);
}

void WriteStream::WriteAlign(size_t alignment, uint8_t paddingByte, uint64_t offset) {
    while ((GetPosition() - offset) % alignment != 0) {
        WriteByte(paddingByte);
    }
}

void WriteStream::WriteStringRaw(std::string_view str, size_t count, bool trim) {
    const char* chars = str.data();
    if (!trim && count > 0 && count < str.size()) {
        throw "String won't fit in provided space!";
    }

    size_t i;
    for (i = 0; i < str.size(); ++i) {
        WriteByte(static_cast<uint8_t>(chars[i]));
    }
    for (; i < count; ++i) {
        WriteByte(0);
    }
}

void WriteStream::WriteString(HyoutaUtils::TextUtils::GameTextEncoding encoding,
                              std::string_view str,
                              size_t count,
                              bool trim) {
    switch (encoding) {
        case HyoutaUtils::TextUtils::GameTextEncoding::ASCII:
            WriteStringRaw(str, count, trim);
            return;
        case HyoutaUtils::TextUtils::GameTextEncoding::UTF8:
            WriteStringRaw(str, count, trim);
            return;
        default: break;
    }
    throw "Writing string not implemented for encoding";
}

void WriteStream::WriteUTF8Nullterm(std::string_view str) {
    WriteStringRaw(str, 0, false);
    WriteByte(0);
}

void WriteStream::WriteShiftJisNullterm(std::string_view str) {
    auto sjis = HyoutaUtils::TextUtils::Utf8ToShiftJis(str.data(), str.size());
    if (!sjis) {
        throw "Invalid UTF-8 string";
    }
    WriteStringRaw(*sjis, 0, false);
    WriteByte(0);
}

void WriteStream::WriteAsciiNullterm(std::string_view str) {
    WriteStringRaw(str, 0, false);
    WriteByte(0);
}

void WriteStream::WriteNulltermString(std::string_view str,
                                      HyoutaUtils::TextUtils::GameTextEncoding encoding) {
    switch (encoding) {
        case HyoutaUtils::TextUtils::GameTextEncoding::ASCII: WriteAsciiNullterm(str); return;
        case HyoutaUtils::TextUtils::GameTextEncoding::ShiftJIS: WriteShiftJisNullterm(str); return;
        case HyoutaUtils::TextUtils::GameTextEncoding::UTF8: WriteUTF8Nullterm(str); return;
        default: break;
    }
    throw "Writing string not implemented for encoding";
}

DuplicatableByteArrayStream::DuplicatableByteArrayStream(const char* data, const size_t length)
  : Data(data), Length(length), CurrentPosition(0) {}

DuplicatableByteArrayStream::~DuplicatableByteArrayStream() = default;

uint64_t DuplicatableByteArrayStream::GetPosition() const {
    return CurrentPosition;
}

void DuplicatableByteArrayStream::SetPosition(uint64_t position) {
    CurrentPosition = position;
}

uint64_t DuplicatableByteArrayStream::GetLength() const {
    return Length;
}

size_t DuplicatableByteArrayStream::Read(char* buffer, size_t count) {
    const size_t pos = CurrentPosition;
    const size_t len = Length;
    if (pos >= len) {
        return 0;
    }

    size_t cnt = len - pos;
    if (cnt > count) {
        cnt = count;
    }

    std::memcpy(buffer, &Data[pos], cnt);
    CurrentPosition += cnt;

    return cnt;
}

int DuplicatableByteArrayStream::ReadByte() {
    if (CurrentPosition < Length) {
        int rv = static_cast<uint8_t>(Data[CurrentPosition]);
        ++CurrentPosition;
        return rv;
    } else {
        return -1;
    }
}

std::unique_ptr<DuplicatableStream> DuplicatableByteArrayStream::Duplicate() const {
    return std::make_unique<DuplicatableByteArrayStream>(Data, Length);
}

void DuplicatableByteArrayStream::ReStart() {
    CurrentPosition = 0;
}

void DuplicatableByteArrayStream::End() {
    CurrentPosition = 0;
}

DuplicatableSharedVectorStream::DuplicatableSharedVectorStream(
    std::shared_ptr<std::vector<char>> data)
  : Data(std::move(data)), CurrentPosition(0) {}

DuplicatableSharedVectorStream::~DuplicatableSharedVectorStream() = default;

uint64_t DuplicatableSharedVectorStream::GetPosition() const {
    return CurrentPosition;
}

void DuplicatableSharedVectorStream::SetPosition(uint64_t position) {
    CurrentPosition = position;
}

uint64_t DuplicatableSharedVectorStream::GetLength() const {
    return Data->size();
}

size_t DuplicatableSharedVectorStream::Read(char* buffer, size_t count) {
    const size_t pos = CurrentPosition;
    const size_t len = Data->size();
    if (pos >= len) {
        return 0;
    }

    size_t cnt = len - pos;
    if (cnt > count) {
        cnt = count;
    }

    std::memcpy(buffer, &(*Data)[pos], cnt);
    CurrentPosition += cnt;

    return cnt;
}

int DuplicatableSharedVectorStream::ReadByte() {
    if (CurrentPosition < Data->size()) {
        int rv = static_cast<uint8_t>((*Data)[CurrentPosition]);
        ++CurrentPosition;
        return rv;
    } else {
        return -1;
    }
}

std::unique_ptr<DuplicatableStream> DuplicatableSharedVectorStream::Duplicate() const {
    return std::make_unique<DuplicatableSharedVectorStream>(Data);
}

void DuplicatableSharedVectorStream::ReStart() {
    CurrentPosition = 0;
}

void DuplicatableSharedVectorStream::End() {
    CurrentPosition = 0;
}

PartialStream::PartialStream(const DuplicatableStream& stream, uint64_t position, uint64_t length)
  : BaseStreamInternal(stream.Duplicate())
  , PartialStart(position)
  , PartialLength(length)
  , CurrentPosition(0)
  , Initialized(false) {
    // C# has an optimization here for chaining partial streams but that needs RTTI,
    // which is not guaranteed to be available depending on compiler settings...
    // if (stream is PartialStream) {
    //    // optimization to better chain partial stream of partial stream
    //    PartialStream parent = stream as PartialStream;
    //    BaseStreamInternal = parent.BaseStreamInternal.Duplicate();
    //    Initialized = false;
    //    PartialStart = parent.PartialStart + position;
    //    PartialLength = length;
    //    CurrentPosition = 0;
    // }
}

PartialStream::~PartialStream() = default;

uint64_t PartialStream::GetPosition() const {
    return CurrentPosition;
}

void PartialStream::SetPosition(uint64_t position) {
    const uint64_t v = position < PartialLength ? position : PartialLength;
    if (!Initialized) {
        BaseStreamInternal->ReStart();
        Initialized = true;
    }
    BaseStreamInternal->SetPosition(PartialStart + v);
    CurrentPosition = v;
}

uint64_t PartialStream::GetLength() const {
    return PartialLength;
}

size_t PartialStream::Read(char* buffer, size_t count) {
    const uint64_t partialBytesLeft = PartialLength - CurrentPosition;
    const size_t c = partialBytesLeft < count ? static_cast<size_t>(partialBytesLeft) : count;
    if (!Initialized) {
        ReStart();
    }
    const size_t v = BaseStreamInternal->Read(buffer, c);
    CurrentPosition += v;
    return v;
}

int PartialStream::ReadByte() {
    const uint64_t partialBytesLeft = PartialLength - CurrentPosition;
    if (partialBytesLeft > 0) {
        if (!Initialized) {
            ReStart();
        }
        int v = BaseStreamInternal->ReadByte();
        if (v != -1) {
            ++CurrentPosition;
        }
        return v;
    }
    return -1;
}

std::unique_ptr<DuplicatableStream> PartialStream::Duplicate() const {
    return std::make_unique<PartialStream>(*BaseStreamInternal, PartialStart, PartialLength);
}

void PartialStream::ReStart() {
    BaseStreamInternal->ReStart();
    BaseStreamInternal->SetPosition(PartialStart);
    CurrentPosition = 0;
    Initialized = true;
}

void PartialStream::End() {
    BaseStreamInternal->End();
    CurrentPosition = 0;
    Initialized = false;
}

MemoryStream::MemoryStream(std::vector<char>& data, size_t initialPosition)
  : Data(data), CurrentPosition(initialPosition) {}

MemoryStream::~MemoryStream() = default;

uint64_t MemoryStream::GetPosition() const {
    return CurrentPosition;
}

void MemoryStream::SetPosition(uint64_t position) {
    CurrentPosition = position;
}

uint64_t MemoryStream::GetLength() const {
    return Data.size();
}

void MemoryStream::Clear() {
    Data.clear();
    CurrentPosition = 0;
}

size_t MemoryStream::Write(const char* buffer, size_t count) {
    while (Data.size() < CurrentPosition) {
        Data.push_back(0);
    }

    for (size_t i = 0; i < count; ++i) {
        WriteByteInternal(buffer[i]);
    }
    return count;
}

void MemoryStream::WriteByte(uint8_t byte) {
    while (Data.size() < CurrentPosition) {
        Data.push_back(0);
    }

    WriteByteInternal(static_cast<char>(byte));
}

void MemoryStream::WriteByteInternal(char byte) {
    if (CurrentPosition < Data.size()) {
        Data[CurrentPosition] = byte;
    } else {
        Data.push_back(byte);
    }
    ++CurrentPosition;
}

void CopyStream(ReadStream& input, WriteStream& output, uint64_t count) {
    std::array<char, 4096> buffer;
    size_t read;

    uint64_t bytesLeft = count;
    while ((read = input.Read(buffer.data(), (size_t)std::min<uint64_t>(buffer.size(), bytesLeft)))
           > 0) {
        if (output.Write(buffer.data(), read) != read) {
            throw "Failed to write bytes to copy";
        }
        bytesLeft -= read;
        if (bytesLeft <= 0) {
            break;
        }
    }

    if (bytesLeft > 0) {
        throw "Failed to read enough bytes for copy";
    }
}
} // namespace HyoutaUtils::Stream
