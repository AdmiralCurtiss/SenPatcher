#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "endian.h"
#include "text.h"

namespace HyoutaUtils::Stream {
// this mostly exists so I can copy-paste code from C# not have to change too much...
struct ReadStream {
    virtual ~ReadStream();
    virtual uint64_t GetPosition() const = 0;
    virtual void SetPosition(uint64_t position) = 0;
    virtual uint64_t GetLength() const = 0;

    virtual size_t Read(char* buffer, size_t count) = 0;
    virtual int ReadByte();

    uint64_t ReadUInt64(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint64_t ReadUInt56(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint64_t ReadUInt48(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint64_t ReadUInt40(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint32_t ReadUInt32(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint32_t ReadUInt24(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint16_t ReadUInt16(HyoutaUtils::EndianUtils::Endianness endian =
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    uint8_t ReadUInt8();

    template<size_t length>
    std::array<char, length> ReadArray() {
        std::array<char, length> rv;
        if (Read(rv.data(), rv.size()) != rv.size()) {
            throw "cannot read data";
        }
        return rv;
    }

    std::vector<char> ReadVector(size_t length);

    void DiscardBytes(size_t length) {
        SetPosition(GetPosition() + length);
    }

    // all of these return UTF8 strings!
    std::string ReadAsciiNulltermFromLocationAndReset(size_t location);
    std::string ReadAsciiNullterm();
    std::string ReadUTF8Nullterm();
    std::string ReadUTF16Nullterm();
    std::string ReadShiftJisNullterm();
    std::string ReadNulltermString(HyoutaUtils::TextUtils::GameTextEncoding encoding);
};

struct WriteStream {
    virtual ~WriteStream();
    virtual uint64_t GetPosition() const = 0;
    virtual void SetPosition(uint64_t position) = 0;
    virtual uint64_t GetLength() const = 0;
    virtual void Clear() = 0;

    virtual size_t Write(const char* buffer, size_t count) = 0;
    virtual void WriteByte(uint8_t byte);

    void WriteUInt64(uint64_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt56(uint64_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt48(uint64_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt40(uint64_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt32(uint32_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt24(uint32_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt16(uint16_t num,
                     HyoutaUtils::EndianUtils::Endianness endian =
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    void WriteUInt8(uint8_t num);

    void WriteAlign(size_t alignment, uint8_t paddingByte = 0, uint64_t offset = 0);

    void WriteStringRaw(std::string_view str, size_t count = 0, bool trim = false);

    // str is assumed to be in UTF8 and will be converted to the given encoding
    void WriteString(HyoutaUtils::TextUtils::GameTextEncoding encoding,
                     std::string_view str,
                     size_t count = 0,
                     bool trim = false);

    void WriteUTF8Nullterm(std::string_view str);
    void WriteShiftJisNullterm(std::string_view str);
    void WriteAsciiNullterm(std::string_view str);
    void WriteNulltermString(std::string_view str,
                             HyoutaUtils::TextUtils::GameTextEncoding encoding);

    void WriteArray(const std::span<const char>& arr) {
        if (Write(arr.data(), arr.size()) != arr.size()) {
            throw "failed to write data";
        }
    }
};

struct DuplicatableStream : public ReadStream {
    // Returns a copy of this stream.
    virtual std::unique_ptr<DuplicatableStream> Duplicate() const = 0;

    // use these two to keep open file handles in check
    // no need to keep thousands of file handles open if we're only ever accessing a handful at
    // once... open or reset underlying stream; call before accessing data
    virtual void ReStart() = 0;

    // signify that we're done with accessing the data for now
    // it should be possible to call ReStart() again later to reopen the stream
    // even if ReStart() is not called, the next access should implicitly
    // start from the beginning of the stream after a call to End()
    virtual void End() = 0;
};

struct DuplicatableByteArrayStream final : public DuplicatableStream {
    DuplicatableByteArrayStream(const char* data, const size_t length);
    DuplicatableByteArrayStream(const DuplicatableByteArrayStream& other) = delete;
    DuplicatableByteArrayStream(DuplicatableByteArrayStream&& other) = delete;
    DuplicatableByteArrayStream& operator=(const DuplicatableByteArrayStream& other) = delete;
    DuplicatableByteArrayStream& operator=(DuplicatableByteArrayStream&& other) = delete;
    ~DuplicatableByteArrayStream() override;

    uint64_t GetPosition() const override;
    void SetPosition(uint64_t position) override;
    uint64_t GetLength() const override;

    size_t Read(char* buffer, size_t count) override;
    int ReadByte() override;

    std::unique_ptr<DuplicatableStream> Duplicate() const override;
    void ReStart() override;
    void End() override;

private:
    const char* Data;
    size_t Length;
    size_t CurrentPosition;
};

struct DuplicatableSharedVectorStream final : public DuplicatableStream {
    DuplicatableSharedVectorStream(std::shared_ptr<std::vector<char>> data);
    DuplicatableSharedVectorStream(const DuplicatableSharedVectorStream& other) = delete;
    DuplicatableSharedVectorStream(DuplicatableSharedVectorStream&& other) = delete;
    DuplicatableSharedVectorStream& operator=(const DuplicatableSharedVectorStream& other) = delete;
    DuplicatableSharedVectorStream& operator=(DuplicatableSharedVectorStream&& other) = delete;
    ~DuplicatableSharedVectorStream() override;

    uint64_t GetPosition() const override;
    void SetPosition(uint64_t position) override;
    uint64_t GetLength() const override;

    size_t Read(char* buffer, size_t count) override;
    int ReadByte() override;

    std::unique_ptr<DuplicatableStream> Duplicate() const override;
    void ReStart() override;
    void End() override;

private:
    std::shared_ptr<std::vector<char>> Data;
    size_t CurrentPosition;
};

struct PartialStream final : public DuplicatableStream {
    PartialStream(const DuplicatableStream& stream, uint64_t position, uint64_t length);
    PartialStream(const PartialStream& other) = delete;
    PartialStream(PartialStream&& other) = delete;
    PartialStream& operator=(const PartialStream& other) = delete;
    PartialStream& operator=(PartialStream&& other) = delete;
    ~PartialStream() override;

    uint64_t GetPosition() const override;
    void SetPosition(uint64_t position) override;
    uint64_t GetLength() const override;

    size_t Read(char* buffer, size_t count) override;
    int ReadByte() override;

    std::unique_ptr<DuplicatableStream> Duplicate() const override;
    void ReStart() override;
    void End() override;

private:
    std::unique_ptr<DuplicatableStream> BaseStreamInternal;
    uint64_t PartialStart;
    uint64_t PartialLength;
    uint64_t CurrentPosition;
    bool Initialized;
};

struct MemoryStream final : public WriteStream {
    explicit MemoryStream(std::vector<char>& data, size_t initialPosition = 0);
    MemoryStream(const MemoryStream& other) = delete;
    MemoryStream(MemoryStream&& other) = delete;
    MemoryStream& operator=(const MemoryStream& other) = delete;
    MemoryStream& operator=(MemoryStream&& other) = delete;
    ~MemoryStream() override;
    uint64_t GetPosition() const override;
    void SetPosition(uint64_t position) override;
    uint64_t GetLength() const override;
    void Clear() override;

    size_t Write(const char* buffer, size_t count) override;
    void WriteByte(uint8_t byte) override;

private:
    void WriteByteInternal(char byte);

public:
    std::vector<char>& Data;
    size_t CurrentPosition;
};

void CopyStream(ReadStream& input, WriteStream& output, uint64_t count);
} // namespace HyoutaUtils::Stream
