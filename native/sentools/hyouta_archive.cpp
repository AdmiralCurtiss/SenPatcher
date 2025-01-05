#include "hyouta_archive.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "zlib/zlib.h"

#include "util/bps.h"
#include "util/memwrite.h"
#include "util/stream.h"
#include "util/text.h"

namespace HyoutaUtils::HyoutaArchive {
namespace Compression {
IHyoutaArchiveCompressionInfo::~IHyoutaArchiveCompressionInfo() = default;

DeflateSharpCompressionInfo::DeflateSharpCompressionInfo(uint64_t uncompressedFilesize)
  : UncompressedFilesize(uncompressedFilesize) {}

DeflateSharpCompressionInfo::~DeflateSharpCompressionInfo() = default;

std::unique_ptr<DeflateSharpCompressionInfo>
    DeflateSharpCompressionInfo::Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                                             uint64_t maxBytes,
                                             HyoutaUtils::EndianUtils::Endianness endian) {
    // note: identifier has already been read
    if (maxBytes < 8) {
        stream.DiscardBytes(maxBytes);
        return nullptr;
    }

    uint64_t uncompressedFilesize = stream.ReadUInt64(endian);
    stream.DiscardBytes(maxBytes - 8);
    return std::make_unique<DeflateSharpCompressionInfo>(uncompressedFilesize);
}

// FIXME: This would be more efficient without the intermediate 'out' buffer.
static bool DecompressDeflateSharpCompression(HyoutaUtils::Stream::ReadStream& inputBuffer,
                                              char* outputBuffer,
                                              size_t outputLength) {
    // adapted from https://zlib.net/zpipe.c which is public domain
    static constexpr size_t CHUNK = 16384;

    int ret;
    unsigned have;
    z_stream strm;
    char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -15);
    if (ret != Z_OK) {
        return false;
    }

    /* decompress until deflate stream ends or end of file */
    char* outputBufferCurrent = outputBuffer;
    size_t outputBufferLeft = outputLength;
    do {
        strm.avail_in = static_cast<uInt>(inputBuffer.Read(in, CHUNK));
        if (strm.avail_in == 0)
            break;
        strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(in));

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR; /* and fall through */
                    [[fallthrough]];
                case Z_DATA_ERROR:
                case Z_MEM_ERROR: (void)inflateEnd(&strm); return false;
            }
            have = CHUNK - strm.avail_out;
            if (have > outputBufferLeft) {
                (void)inflateEnd(&strm);
                return false;
            }
            std::memcpy(outputBufferCurrent, out, have);
            outputBufferCurrent += have;
            outputBufferLeft -= have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END) {
        return false;
    }
    return outputBufferLeft == 0;
}

std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream>
    DeflateSharpCompressionInfo::Decompress(HyoutaUtils::Stream::ReadStream& data) const {
    std::vector<char> ms;
    ms.resize(UncompressedFilesize);
    if (!DecompressDeflateSharpCompression(data, ms.data(), ms.size())) {
        throw "decompression error";
    }
    return std::make_unique<HyoutaUtils::Stream::DuplicatableSharedVectorStream>(
        std::make_shared<std::vector<char>>(std::move(ms)));
}

uint32_t DeflateSharpCompressionInfo::MaximumCompressionInfoLength() const {
    return 16u; // identifier + uncompressed filesize
}

std::unique_ptr<IHyoutaArchiveCompressionInfo>
    Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                uint64_t maxBytes,
                HyoutaUtils::EndianUtils::Endianness endian) {
    if (maxBytes < 8) {
        stream.DiscardBytes(maxBytes);
        return nullptr;
    }

    uint64_t identifier = stream.ReadUInt64(EndianUtils::Endianness::BigEndian);
    switch (identifier) {
        case 0:
            // archive has compression, but this file is not compressed
            stream.DiscardBytes(maxBytes - 8);
            return nullptr;
        case DeflateSharpCompressionInfo::Identifier:
            return DeflateSharpCompressionInfo::Deserialize(stream, maxBytes - 8, endian);
        default:
            // Console.WriteLine("Unknown compression type: " + identifier.ToString("x16"));
            stream.DiscardBytes(maxBytes - 8);
            return nullptr;
    }
}
} // namespace Compression

HyoutaArchiveBpsPatchInfo::HyoutaArchiveBpsPatchInfo(uint64_t index,
                                                     uint64_t targetFilesize,
                                                     HyoutaArchiveChunk* referencedChunk)
  : FileIndexToPatch(index), TargetFilesize(targetFilesize), ReferencedChunk(referencedChunk) {}

std::optional<HyoutaArchiveBpsPatchInfo>
    HyoutaArchiveBpsPatchInfo::Deserialize(HyoutaUtils::Stream::ReadStream& stream,
                                           uint64_t maxBytes,
                                           HyoutaUtils::EndianUtils::Endianness endian,
                                           uint64_t currentFileIndex,
                                           HyoutaArchiveChunk* referencedChunk) {
    if (maxBytes < 16) {
        stream.DiscardBytes(maxBytes);
        return std::nullopt;
    } else {
        uint64_t fileIndexToPatch = stream.ReadUInt64(endian);
        uint64_t targetFilesize = stream.ReadUInt64(endian);
        stream.DiscardBytes(maxBytes - 16);
        if (fileIndexToPatch == currentFileIndex) {
            // this is how you set a file to be unpatched in an archive that has patches
            return std::nullopt;
        } else {
            return HyoutaArchiveBpsPatchInfo(fileIndexToPatch, targetFilesize, referencedChunk);
        }
    }
}

static std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream>
    ApplyBpsPatch(const HyoutaArchiveBpsPatchInfo& patchInfo,
                  HyoutaUtils::Stream::ReadStream& data) {
    const HyoutaArchiveFileInfo& sourceFile =
        patchInfo.ReferencedChunk->Files[patchInfo.FileIndexToPatch];
    auto sourceStream = sourceFile.GetDataStream();
    sourceStream->SetPosition(0);
    data.SetPosition(0);
    std::vector<char> ms;
    ms.reserve(patchInfo.TargetFilesize);
    HyoutaUtils::Bps::ApplyPatchToStream(*sourceStream, data, ms);
    return std::make_unique<HyoutaUtils::Stream::DuplicatableSharedVectorStream>(
        std::make_shared<std::vector<char>>(std::move(ms)));
}

std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream>
    HyoutaArchiveFileInfo::GetDataStream() const {
    std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream> s = Data->Duplicate();
    if (CompressionInfo != nullptr && StreamIsCompressed) {
        s = CompressionInfo->Decompress(*s);
    }
    if (BpsPatchInfo.has_value() && StreamIsBpsPatch) {
        s = ApplyBpsPatch(*BpsPatchInfo, *s);
    }
    return s;
}

static uint32_t ReadContentLength(HyoutaUtils::Stream::DuplicatableStream& dataBlockStream,
                                  HyoutaUtils::EndianUtils::Endianness e) {
    uint16_t l = dataBlockStream.ReadUInt16(e);
    return l == 0 ? 0x10000 : (uint32_t)l;
}

static std::optional<std::string> ReadString(HyoutaUtils::Stream::DuplicatableStream& s,
                                             uint32_t maxBytes,
                                             HyoutaUtils::EndianUtils::Endianness e) {
    if (maxBytes < 8) {
        // can't be a valid string
        s.DiscardBytes(maxBytes);
        return std::nullopt;
    }

    uint64_t rawlength = s.ReadUInt64(e);
    uint64_t length = (rawlength & 0x7fffffffffffffffull);
    bool hasOffset = (rawlength & 0x8000000000000000ull) > 0;

    if (hasOffset) {
        // format is 8 bytes length, then 8 bytes position of string in data
        if (maxBytes < 16) {
            // can't be valid
            s.DiscardBytes(maxBytes - 8);
            return std::nullopt;
        }

        uint64_t offset = s.ReadUInt64(e);
        uint64_t p = s.GetPosition() + (maxBytes - 16u);
        s.SetPosition(offset);
        std::string str;
        str.resize(length);
        s.Read(str.data(), length);
        s.SetPosition(p);
        return str;
    } else {
        // format is 8 bytes length, then [number read] bytes string
        uint32_t restBytes = maxBytes - 8;
        if (length > restBytes) {
            // can't be a valid string
            s.DiscardBytes(restBytes);
            return std::nullopt;
        }

        std::string str;
        str.resize(length);
        s.Read(str.data(), length);
        s.DiscardBytes(restBytes - length);
        return str;
    }
}

static void ReadAlign(HyoutaUtils::Stream::DuplicatableStream& s, uint64_t alignment) {
    uint64_t pos = s.GetPosition();
    while ((pos % alignment) != 0) {
        s.DiscardBytes(1);
        ++pos;
    }
}

HyoutaArchiveChunk::HyoutaArchiveChunk(HyoutaUtils::Stream::DuplicatableStream& duplicatableStream,
                                       uint64_t* chunkLength) {
    auto data = duplicatableStream.Duplicate();
    data->SetPosition(0);

    // header
    uint64_t extraMagic = data->ReadUInt64(EndianUtils::Endianness::LittleEndian);
    uint64_t magic = extraMagic & 0x00fffffffffffffful;
    if (magic != 0x6b6e7568636168) {
        throw "wrong magic";
    }
    uint8_t extra = (uint8_t)((extraMagic >> 56) & 0xffu);
    uint8_t packedAlignment = (uint8_t)(extra & 0x1fu);
    uint64_t unpackedAlignment = 1ull << packedAlignment;
    bool hasMetadata = (extra & 0x20) != 0;
    bool isCompressed = (extra & 0x40) != 0;
    bool isBigEndian = (extra & 0x80) != 0;
    EndianUtils::Endianness e =
        isBigEndian ? EndianUtils::Endianness::BigEndian : EndianUtils::Endianness::LittleEndian;
    uint64_t endOfFileOffset = data->ReadUInt64(e) << packedAlignment;
    uint64_t tableOfContentsOffset = data->ReadUInt64(e) << packedAlignment;
    uint64_t filecount = data->ReadUInt64(e);
    *chunkLength = endOfFileOffset;

    if (hasMetadata) {
        // just skip past this for now
        uint64_t metadataLength = data->ReadUInt64(e);
        data->DiscardBytes(metadataLength);
    }

    std::unique_ptr<HyoutaUtils::Stream::DuplicatableStream> dataBlockStream;
    if (isCompressed) {
        uint16_t compressionInfoLengthRaw = data->ReadUInt16(e);
        uint32_t compressionInfoLength = compressionInfoLengthRaw & 0xfffcu;
        int compressionInfoAlignmentPacked = (compressionInfoLengthRaw & 0x3) + 1;
        ReadAlign(*data, 1ull << compressionInfoAlignmentPacked);
        auto compressionInfo = Compression::Deserialize(
            *data, compressionInfoLength == 0 ? 0x10000u : compressionInfoLength, e);
        if (!compressionInfo) {
            throw "could not read compression type";
        }
        HyoutaUtils::Stream::PartialStream ps(
            *data, data->GetPosition(), endOfFileOffset - data->GetPosition());
        dataBlockStream = compressionInfo->Decompress(ps);
    } else {
        ReadAlign(*data, unpackedAlignment);
        dataBlockStream = std::make_unique<HyoutaUtils::Stream::PartialStream>(
            *data, data->GetPosition(), endOfFileOffset - data->GetPosition());
    }

    dataBlockStream->SetPosition(tableOfContentsOffset);
    uint32_t offsetToFirstFileInfo = ReadContentLength(*dataBlockStream, e);

    // decode content bitfield(s)
    uint64_t numberOfUnknownBits = 0;
    uint16_t contentBitfield1 = dataBlockStream->ReadUInt16(e);
    bool hasDummyContent = (contentBitfield1 & 0x0001u) != 0;
    bool hasFilename = (contentBitfield1 & 0x0002u) != 0;
    bool hasCompression = (contentBitfield1 & 0x0004u) != 0;
    bool hasBpsPatch = (contentBitfield1 & 0x0008u) != 0;
    bool hasCrc32 = (contentBitfield1 & 0x0010u) != 0;
    bool hasMd5 = (contentBitfield1 & 0x0020u) != 0;
    bool hasSha1 = (contentBitfield1 & 0x0040u) != 0;
    numberOfUnknownBits += (contentBitfield1 & 0x0080u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x0100u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x0200u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x0400u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x0800u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x1000u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x2000u) != 0 ? 1 : 0;
    numberOfUnknownBits += (contentBitfield1 & 0x4000u) != 0 ? 1 : 0;
    uint16_t currentBitfield = contentBitfield1;
    while ((currentBitfield & 0x8000u) != 0) {
        // more bitfields, though we don't understand them since only the first handful of
        // bits are defined at the moment, so just count and skip them
        currentBitfield = dataBlockStream->ReadUInt16(e);
        numberOfUnknownBits += (currentBitfield & 0x0001u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0002u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0004u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0008u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0010u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0020u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0040u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0080u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0100u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0200u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0400u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x0800u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x1000u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x2000u) != 0 ? 1 : 0;
        numberOfUnknownBits += (currentBitfield & 0x4000u) != 0 ? 1 : 0;
    }
    uint32_t dummyContentLength = hasDummyContent ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t filenameLength = hasFilename ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t compressionLength = hasCompression ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t bpspatchLength = hasBpsPatch ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t crc32Length = hasCrc32 ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t md5Length = hasMd5 ? ReadContentLength(*dataBlockStream, e) : 0;
    uint32_t sha1Length = hasSha1 ? ReadContentLength(*dataBlockStream, e) : 0;
    uint64_t unknownContentLength = 0;
    for (uint64_t i = 0; i < numberOfUnknownBits; ++i) {
        unknownContentLength += ReadContentLength(*dataBlockStream, e);
    }

    dataBlockStream->SetPosition(tableOfContentsOffset + offsetToFirstFileInfo);
    std::vector<HyoutaArchiveFileInfo> files;
    files.reserve(filecount);
    for (uint64_t i = 0; i < filecount; ++i) {
        uint64_t offset = dataBlockStream->ReadUInt64(e) << packedAlignment;
        uint64_t filesize = dataBlockStream->ReadUInt64(e);
        HyoutaArchiveFileInfo fi;
        if (hasDummyContent) {
            dataBlockStream->DiscardBytes(dummyContentLength);
        }
        if (hasFilename) {
            auto fn = ReadString(*dataBlockStream, filenameLength, e);
            if (fn) {
                fi.Filename = std::move(*fn);
            }
        }
        if (hasCompression) {
            fi.CompressionInfo = Compression::Deserialize(*dataBlockStream, compressionLength, e);
            fi.StreamIsCompressed = true;
        }
        if (hasBpsPatch) {
            fi.BpsPatchInfo = HyoutaArchiveBpsPatchInfo::Deserialize(
                *dataBlockStream, bpspatchLength, e, i, this);
            fi.StreamIsBpsPatch = fi.BpsPatchInfo.has_value();
        }
        if (hasCrc32) {
            if (crc32Length >= 4) {
                fi.crc32 = dataBlockStream->ReadUInt32(EndianUtils::Endianness::BigEndian);
                dataBlockStream->DiscardBytes(crc32Length - 4);
            } else {
                dataBlockStream->DiscardBytes(crc32Length);
            }
        }
        if (hasMd5) {
            if (md5Length >= 16) {
                fi.md5 = HyoutaUtils::Hash::MD5{.Hash = dataBlockStream->ReadArray<16>()};
                dataBlockStream->DiscardBytes(md5Length - 16);
            } else {
                dataBlockStream->DiscardBytes(md5Length);
            }
        }
        if (hasSha1) {
            if (sha1Length >= 20) {
                fi.sha1 = HyoutaUtils::Hash::SHA1{.Hash = dataBlockStream->ReadArray<20>()};
                dataBlockStream->DiscardBytes(sha1Length - 20);
            } else {
                dataBlockStream->DiscardBytes(sha1Length);
            }
        }
        dataBlockStream->DiscardBytes(unknownContentLength);

        fi.Data = std::make_unique<HyoutaUtils::Stream::PartialStream>(
            *dataBlockStream, offset, filesize);
        files.push_back(std::move(fi));
    }

    Files = std::move(files);
}

uint64_t HyoutaArchiveContainer::GetFilecount() const {
    uint64_t c = 0;
    for (const HyoutaArchiveChunk& chunk : Chunks) {
        c += chunk.Files.size();
    }
    return c;
}

HyoutaArchiveContainer::HyoutaArchiveContainer(HyoutaUtils::Stream::DuplicatableStream& stream) {
    uint64_t totalChunkLengths = 0;
    Chunks.emplace_back(stream, &totalChunkLengths);
    const uint64_t streamLength = stream.GetLength();
    while (totalChunkLengths < streamLength) {
        uint64_t tmp = 0;
        HyoutaUtils::Stream::PartialStream ps(
            stream, totalChunkLengths, streamLength - totalChunkLengths);
        Chunks.emplace_back(ps, &tmp);
        totalChunkLengths += tmp;
    }

    Chunks.shrink_to_fit();
    FileCountOffsets.resize(Chunks.size() + 1);
    FileCountOffsets[0] = 0;
    for (size_t i = 0; i < Chunks.size(); ++i) {
        FileCountOffsets[i + 1] = FileCountOffsets[i] + Chunks[i].Files.size();
    }
}

HyoutaArchiveFileInfo* HyoutaArchiveContainer::GetFile(uint64_t index) {
    // is this right???
    for (size_t i = 1; i < FileCountOffsets.size(); ++i) {
        if (index < FileCountOffsets[i]) {
            return &Chunks.at(i - 1).Files.at(index - FileCountOffsets[i - 1]);
        }
    }
    return nullptr;
}
} // namespace HyoutaUtils::HyoutaArchive
