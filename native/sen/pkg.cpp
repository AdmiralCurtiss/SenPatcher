#include "pkg.h"

#include "util/endian.h"
#include "util/stream.h"

namespace SenLib {
Pkg::Pkg(const char* buffer, size_t length, HyoutaUtils::EndianUtils::Endianness e) {
    SenLib::DuplicatableByteArrayStream s(buffer, length);
    Unknown = s.ReadUInt32(e);
    size_t fileCount = s.ReadUInt32(e);

    Files.reserve(fileCount);
    for (size_t i = 0; i < fileCount; ++i) {
        auto& f = Files.emplace_back(s, e);
        if (f.CompressedSize < length && f.DataPosition <= length - f.CompressedSize) {
            f.Data = buffer + f.DataPosition;
        }
    }
}

void Pkg::WriteToStream(SenLib::WriteStream& s, HyoutaUtils::EndianUtils::Endianness e) const {
    uint64_t pos = s.GetPosition();
    s.WriteUInt32(Unknown, e);
    s.WriteUInt32((uint32_t)Files.size(), e);
    for (size_t i = 0; i < Files.size(); ++i) {
        auto& f = Files[i];
        s.WriteArray(f.Filename);
        s.WriteUInt32(f.UncompressedSize, e);
        s.WriteUInt32(f.CompressedSize, e);
        s.WriteUInt32(0, e); // file position, fill in later
        s.WriteUInt32(f.Flags, e);
    }

    for (size_t i = 0; i < Files.size(); ++i) {
        auto& f = Files[i];
        uint64_t currentFileStart = s.GetPosition();
        uint64_t p = currentFileStart - pos;
        s.SetPosition(pos + 8 + 0x50 * static_cast<uint64_t>(i) + 0x48);
        s.WriteUInt32(p, e);
        s.SetPosition(currentFileStart);

        if (!f.Data) {
            throw "data not set";
        }

        DuplicatableByteArrayStream fs(f.Data, f.CompressedSize);
        SenLib::CopyStream(fs, s, f.CompressedSize);
    }
}

PkgFile::PkgFile(SenLib::ReadStream& s, HyoutaUtils::EndianUtils::Endianness e) {
    Filename = s.ReadArray<0x40>();
    UncompressedSize = s.ReadUInt32(e); // 0x40
    CompressedSize = s.ReadUInt32(e);   // 0x44
    DataPosition = s.ReadUInt32(e);     // 0x48
    Flags = s.ReadUInt32(e);            // 0x4c
}
} // namespace SenLib
