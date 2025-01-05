#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "util/endian.h"
#include "util/hash/crc32.h"
#include "util/hash/md5.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

// dumb overcomplicated archive format I made years ago, we need to support reading it to allow
// unpatching from pre 1.0 senpatcher versions...

namespace HyoutaUtils::HyoutaArchive {
namespace Compression {
struct IHyoutaArchiveCompressionInfo {
    virtual std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream>
        Decompress(HyoutaUtils::Stream::ReadStream& data) const = 0;
    virtual uint32_t MaximumCompressionInfoLength() const = 0;
    virtual ~IHyoutaArchiveCompressionInfo();
};
struct DeflateSharpCompressionInfo : public IHyoutaArchiveCompressionInfo {
    // this was originally using the built-in C# DeflateStream, stay compatible with that
    DeflateSharpCompressionInfo(uint64_t uncompressedFilesize);
    DeflateSharpCompressionInfo(const DeflateSharpCompressionInfo& other) = delete;
    DeflateSharpCompressionInfo(DeflateSharpCompressionInfo&& other) = delete;
    DeflateSharpCompressionInfo& operator=(const DeflateSharpCompressionInfo& other) = delete;
    DeflateSharpCompressionInfo& operator=(DeflateSharpCompressionInfo&& other) = delete;
    ~DeflateSharpCompressionInfo() override;

    static std::unique_ptr<DeflateSharpCompressionInfo>
        Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                    uint64_t maxBytes,
                    HyoutaUtils::EndianUtils::Endianness endian);

    std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream>
        Decompress(HyoutaUtils::Stream::ReadStream& data) const override;
    uint32_t MaximumCompressionInfoLength() const override;

    static constexpr uint64_t Identifier = 0x6465666C61746523u;
    uint64_t UncompressedFilesize = 0;
};

std::unique_ptr<IHyoutaArchiveCompressionInfo>
    Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                uint64_t maxBytes,
                HyoutaUtils::EndianUtils::Endianness endian);
} // namespace Compression

struct HyoutaArchiveChunk;

struct HyoutaArchiveBpsPatchInfo {
    uint64_t FileIndexToPatch;
    uint64_t TargetFilesize;
    HyoutaArchiveChunk* ReferencedChunk;

    HyoutaArchiveBpsPatchInfo(uint64_t index,
                              uint64_t targetFilesize,
                              HyoutaArchiveChunk* referencedChunk);

    static std::optional<HyoutaArchiveBpsPatchInfo>
        Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                    uint64_t maxBytes,
                    HyoutaUtils::EndianUtils::Endianness endian,
                    uint64_t currentFileIndex,
                    HyoutaArchiveChunk* referencedChunk);
};

struct HyoutaArchiveFileInfo { // : public IFile
    // the actual data of this file
    // this stream may be compressed, and it may be a bps patch that needs to be applied first
    // if you just want the file data and don't care about the internal state use DataStream instead
    std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream> Data;

    std::string Filename;

    // if CompressionInfo is not null but StreamIsCompressed is false, then the file is intended to
    // be compressed within the on-disk container but has already been decompressed (or not yet
    // compressed) in memory if packing with such a file info, we need to compress the data before
    // writing it into the archive if StreamIsCompressed is true on packing we can assume the data
    // was already compressed and can just write it as-is
    std::unique_ptr<Compression::IHyoutaArchiveCompressionInfo> CompressionInfo;
    bool StreamIsCompressed = false;

    // just like CompressionInfo above
    // if a stream is both compressed and a patch:
    // unpacking needs to first decompress and then patch-apply, packing is the other way around
    std::optional<HyoutaArchiveBpsPatchInfo> BpsPatchInfo;
    bool StreamIsBpsPatch = false;

    std::optional<uint32_t> crc32;
    std::optional<HyoutaUtils::Hash::MD5> md5;
    std::optional<HyoutaUtils::Hash::SHA1> sha1;

    std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream> GetDataStream() const;
};

struct HyoutaArchiveChunk { // : public IContainer
    std::vector<HyoutaArchiveFileInfo> Files;

    HyoutaArchiveChunk(HyoutaUtils::Stream::DuplicatableStream& duplicatableStream,
                       uint64_t* chunkLength);
};

struct HyoutaArchiveContainer { // : public IContainer
    std::vector<HyoutaArchiveChunk> Chunks;
    std::vector<uint64_t> FileCountOffsets;

    uint64_t GetFilecount() const;

    HyoutaArchiveContainer(HyoutaUtils::Stream::DuplicatableStream& stream);

    HyoutaArchiveFileInfo* GetFile(uint64_t index);
};
} // namespace HyoutaUtils::HyoutaArchive
