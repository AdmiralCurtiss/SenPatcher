#include "bps.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "util/hash/crc32.h"
#include "util/endian.h"
#include "util/stream.h"

namespace HyoutaUtils::Bps {
namespace {
enum class Action {
    SourceRead,
    TargetRead,
    SourceCopy,
    TargetCopy,
};

uint64_t ReadUnsignedNumber(HyoutaUtils::Stream::ReadStream& s) {
    uint64_t data = 0;
    uint64_t shift = 1;
    while (true) {
        int x = s.ReadByte();
        if (x == -1) {
            throw "Reached end of stream while decoding BPS number.";
        }
        data += ((uint64_t)(x & 0x7f)) * shift;
        if ((x & 0x80) != 0) {
            break;
        }
        shift <<= 7;
        data += shift;
    }
    return data;
}

int64_t ReadSignedNumber(HyoutaUtils::Stream::ReadStream& s) {
    uint64_t n = ReadUnsignedNumber(s);
    int64_t v = (int64_t)(n >> 1);
    return ((n & 1) != 0) ? -v : v;
}

std::pair<Action, uint64_t> ReadAction(HyoutaUtils::Stream::ReadStream& s) {
    uint64_t n = ReadUnsignedNumber(s);
    uint64_t command = n & 3;
    uint64_t length = (n >> 2) + 1;
    switch (command) {
        case 0: return std::make_pair(Action::SourceRead, length);
        case 1: return std::make_pair(Action::TargetRead, length);
        case 2: return std::make_pair(Action::SourceCopy, length);
        case 3: return std::make_pair(Action::TargetCopy, length);
    }
    throw "should never reach here";
}

uint32_t CalculateCRC32FromCurrentPosition(HyoutaUtils::Stream::ReadStream& s, uint64_t bytecount) {
    uint32_t crc32 = crc_init();
    std::array<char, 4096> buffer;
    size_t read;

    uint64_t bytesLeft = bytecount;
    while ((read = s.Read(buffer.data(), (size_t)std::min<uint64_t>(buffer.size(), bytesLeft)))
           > 0) {
        crc32 = crc_update(crc32, buffer.data(), read);
        bytesLeft -= read;
        if (bytesLeft <= 0) {
            break;
        }
    }

    if (bytesLeft > 0) {
        throw "Failed to read enough bytes for checksum";
    }

    crc32 = crc_finalize(crc32);
    return crc32;
}

uint32_t CalculateCRC32(const char* data, size_t length) {
    uint32_t crc32 = crc_init();
    crc32 = crc_update(crc32, data, length);
    crc32 = crc_finalize(crc32);
    return crc32;
}

struct BpsPatcher {
    HyoutaUtils::Stream::ReadStream& Source;
    HyoutaUtils::Stream::ReadStream& Patch;
    HyoutaUtils::Stream::MemoryStream& Target;

    uint64_t SourceRelativeOffset = 0;
    uint64_t TargetRelativeOffset = 0;

    BpsPatcher(HyoutaUtils::Stream::ReadStream& source, HyoutaUtils::Stream::ReadStream& patch, HyoutaUtils::Stream::MemoryStream& target)
      : Source(source), Patch(patch), Target(target) {}

    BpsPatcher(const BpsPatcher& other) = delete;
    BpsPatcher(BpsPatcher&& other) = delete;
    BpsPatcher& operator=(const BpsPatcher& other) = delete;
    BpsPatcher& operator=(BpsPatcher&& other) = delete;

    ~BpsPatcher() = default;

    void ApplyPatchToStreamInternal() {
        Patch.SetPosition(0);
        Target.SetPosition(0);

        uint64_t patchSize = Patch.GetLength();
        if (patchSize < 12) {
            throw "Patch too small to be a valid patch.";
        }
        uint64_t patchFooterPosition = patchSize - 12;

        // note: spec doesn't actually say what endian, but files in the wild suggest little
        Patch.SetPosition(patchFooterPosition);
        uint32_t checksumSource =
            Patch.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        uint32_t checksumTarget =
            Patch.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        uint32_t checksumPatch =
            Patch.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        Patch.SetPosition(0);
        uint32_t actualChecksumPatch = CalculateCRC32FromCurrentPosition(Patch, patchSize - 4);
        if (checksumPatch != actualChecksumPatch) {
            throw "Patch checksum incorrect.";
        }

        Patch.SetPosition(0);
        uint32_t magic = Patch.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        if (magic != 0x31535042) {
            throw "Wrong patch magic.";
        }

        uint64_t sourceSize = ReadUnsignedNumber(Patch);
        if (sourceSize != (uint64_t)Source.GetLength()) {
            throw "Source size mismatch.";
        }
        uint64_t targetSize = ReadUnsignedNumber(Patch);
        uint64_t metadataSize = ReadUnsignedNumber(Patch);
        if (metadataSize > 0) {
            // skip metadata, we don't care about it
            Patch.SetPosition(Patch.GetPosition() + metadataSize);
        }

        Source.SetPosition(0);
        uint32_t actualChecksumSource =
            CalculateCRC32FromCurrentPosition(Source, Source.GetLength());
        if (checksumSource != actualChecksumSource) {
            throw "Source checksum incorrect.";
        }

        Source.SetPosition(0);
        Target.SetPosition(0);
        while (Patch.GetPosition() < patchFooterPosition) {
            auto action = ReadAction(Patch);
            switch (action.first) {
                case Action::SourceRead: DoSourceRead(action.second); break;
                case Action::TargetRead: DoTargetRead(action.second); break;
                case Action::SourceCopy: DoSourceCopy(action.second); break;
                case Action::TargetCopy: DoTargetCopy(action.second); break;
            }
        }

        if (Patch.GetPosition() != patchFooterPosition) {
            throw "Malformed action stream.";
        }

        if ((uint64_t)Target.GetPosition() != targetSize) {
            throw "Target size incorrect.";
        }

        Target.SetPosition(0);
        uint32_t actualChecksumTarget = CalculateCRC32(Target.Data.data(), Target.Data.size());
        if (checksumTarget != actualChecksumTarget) {
            throw "Target checksum incorrect.";
        }
    }

    void DoSourceRead(uint64_t length) {
        if (Source.GetLength() < Target.GetPosition()) {
            throw "Invalid length in SourceRead.";
        }
        Source.SetPosition(Target.GetPosition());
        if (((uint64_t)Source.GetPosition()) + length > (uint64_t)Source.GetLength()) {
            throw "Invalid length in SourceRead.";
        }
        HyoutaUtils::Stream::CopyStream(Source, Target, length);
    }

    void DoTargetRead(uint64_t length) {
        if (((uint64_t)Patch.GetPosition()) + length > (uint64_t)Patch.GetLength()) {
            throw "Invalid length in TargetRead.";
        }
        HyoutaUtils::Stream::CopyStream(Patch, Target, length);
    }

    void AddChecked(uint64_t& pos, int64_t d, uint64_t length) {
        // pos must end up in range [0, length-1], else error out
        if (d >= 0) {
            // d is positive, make sure we don't end up >= length
            uint64_t o = (uint64_t)d;
            if (o >= (length - pos)) {
                throw "Invalid offset.";
            }
            pos += o;
        } else {
            // d is negative, make sure we don't end up < 0
            uint64_t o = (uint64_t)(-d);
            if (o > pos) {
                throw "Invalid offset.";
            }
            pos -= o;
        }
    }

    void DoSourceCopy(uint64_t length) {
        int64_t d = ReadSignedNumber(Patch);
        AddChecked(SourceRelativeOffset, d, Source.GetLength());
        if (SourceRelativeOffset + length > (uint64_t)Source.GetLength()) {
            throw "Invalid length in SourceCopy.";
        }
        Source.SetPosition(SourceRelativeOffset);
        HyoutaUtils::Stream::CopyStream(Source, Target, length);
        SourceRelativeOffset += length;
    }

    void DoTargetCopy(uint64_t length) {
        int64_t d = ReadSignedNumber(Patch);
        AddChecked(TargetRelativeOffset, d, Target.GetLength());
        for (uint64_t i = 0; i < length; ++i) {
            uint64_t p = Target.GetPosition();
            Target.SetPosition(TargetRelativeOffset);
            ++TargetRelativeOffset;
            uint8_t b = static_cast<uint8_t>(Target.Data[Target.CurrentPosition]);
            Target.SetPosition(p);
            Target.WriteByte(b);
        }
    }
};
} // namespace

void ApplyPatchToStream(HyoutaUtils::Stream::ReadStream& source,
                        HyoutaUtils::Stream::ReadStream& patch,
                        std::vector<char>& target) {
    target.clear();
    HyoutaUtils::Stream::MemoryStream ms(target);
    BpsPatcher patcher(source, patch, ms);
    patcher.ApplyPatchToStreamInternal();
}
} // namespace HyoutaUtils::Bps
