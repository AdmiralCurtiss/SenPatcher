#include "bra.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>

#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

static std::string_view StripToNull(std::string_view sv) {
    for (size_t i = 0; i < sv.size(); ++i) {
        if (sv[i] == '\0') {
            return sv.substr(0, i);
        }
    }
    return sv;
}

namespace SenPatcher {
BRA::BRA() = default;

BRA::BRA(BRA&& other)
  : FileCount(other.FileCount)
  , FileInfo(std::move(other.FileInfo))
  , FileHandle(std::move(other.FileHandle)) {
    other.FileCount = 0;
}

BRA& BRA::operator=(BRA&& other) {
    this->Clear();
    this->FileCount = other.FileCount;
    this->FileInfo = std::move(other.FileInfo);
    this->FileHandle = std::move(other.FileHandle);
    other.FileCount = 0;
    return *this;
}

BRA::~BRA() {
    Clear();
}

void BRA::Clear() {
    FileCount = 0;
    FileInfo.reset();
    FileHandle.Close();
}

bool BRA::Load(std::string_view path) {
    this->Clear();

    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    return Load(f);
}

bool BRA::Load(HyoutaUtils::IO::File& f) {
    using namespace HyoutaUtils::MemRead;

    if (!f.IsOpen()) {
        return false;
    }
    auto filesize = f.GetLength();
    if (!filesize) {
        return false;
    }

    std::array<char, 0x10> header;
    if (f.Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }

    // magic + version
    if (!(header[0] == 0x50 && header[1] == 0x44 && header[2] == 0x41 && header[3] == 0x00
          && header[4] == 0x02 && header[5] == 0x00 && header[6] == 0x00 && header[7] == 0x00)) {
        return false;
    }

    const uint32_t footerPosition = ReadUInt32(&header[0x8]);
    const uint32_t fileCount = ReadUInt32(&header[0xc]);
    if (footerPosition >= *filesize) {
        return false;
    }
    if (!f.SetPosition(footerPosition)) {
        return false;
    }

    const uint64_t footerSize = (*filesize - footerPosition);

    auto fileInfos = std::make_unique<BRAFileInfo[]>(fileCount);
    if (!fileInfos) {
        return false;
    }

    uint32_t realFileCount = 0;
    {
        auto footerMemory = std::make_unique_for_overwrite<char[]>(footerSize);
        if (!footerMemory) {
            return false;
        }
        if (f.Read(footerMemory.get(), footerSize) != footerSize) {
            return false;
        }

        uint64_t offset = 0;
        for (size_t i = 0; i < fileCount; ++i) {
            if (offset + 0x18 > footerSize) {
                break;
            }

            const uint32_t compressedSize = ReadUInt32(&footerMemory[offset + 0x8]);
            const uint32_t uncompressedSize = ReadUInt32(&footerMemory[offset + 0xc]);
            const uint16_t pathLength = ReadUInt16(&footerMemory[offset + 0x10]);
            const uint32_t dataPosition = ReadUInt32(&footerMemory[offset + 0x14]);
            offset += 0x18;

            if (offset + pathLength > footerSize) {
                break;
            }

            const std::string_view path(&footerMemory[offset], pathLength);
            offset += pathLength;

            auto pathShiftJis = StripToNull(path);
            auto pathUtf8 =
                HyoutaUtils::TextUtils::ShiftJisToUtf8(pathShiftJis.data(), pathShiftJis.size());
            if (!pathUtf8) {
                return false;
            }

            auto& fileInfo = fileInfos[i];
            if (pathUtf8->size() > fileInfo.Path.size()) {
                return false;
            }

            fileInfo.CompressedSize = compressedSize;
            fileInfo.UncompressedSize = uncompressedSize;
            fileInfo.DataPosition = dataPosition;
            std::memcpy(fileInfo.Path.data(), pathUtf8->data(), pathUtf8->size());

            ++realFileCount;
        }
    }

    this->FileCount = realFileCount;
    this->FileInfo = std::move(fileInfos);
    this->FileHandle = std::move(f);

    return true;
}
} // namespace SenPatcher
