#include "p3a.h"

#include <cstdint>
#include <filesystem>
#include <memory>

#include "util/file.h"

#include "lz4/lz4.h"

#include "zstd/common/xxhash.h"
#include "zstd/zstd.h"

#include "p3a/structs.h"

namespace SenPatcher {
namespace {
struct ZSTD_DDict_Deleter {
    void operator()(ZSTD_DDict* ptr) {
        if (ptr) {
            ZSTD_freeDDict(ptr);
        }
    }
};
using ZSTD_DDict_UniquePtr = std::unique_ptr<ZSTD_DDict, ZSTD_DDict_Deleter>;

struct ZSTD_DCtx_Deleter {
    void operator()(ZSTD_DCtx* ptr) {
        if (ptr) {
            ZSTD_freeDCtx(ptr);
        }
    }
};
using ZSTD_DCtx_UniquePtr = std::unique_ptr<ZSTD_DCtx, ZSTD_DCtx_Deleter>;
} // namespace

P3A::P3A() = default;

P3A::P3A(P3A&& other)
  : FileCount(other.FileCount)
  , FileInfo(std::move(other.FileInfo))
  , Dict(other.Dict)
  , FileHandle(std::move(other.FileHandle)) {
    other.FileCount = 0;
    other.Dict = nullptr;
}

P3A& P3A::operator=(P3A&& other) {
    this->Clear();
    this->FileCount = other.FileCount;
    this->FileInfo = std::move(other.FileInfo);
    this->Dict = other.Dict;
    this->FileHandle = std::move(other.FileHandle);
    other.FileCount = 0;
    other.Dict = nullptr;
    return *this;
}

P3A::~P3A() {
    Clear();
}

void P3A::Clear() {
    FileCount = 0;
    FileInfo.reset();
    FileHandle.Close();
    if (Dict) {
        ZSTD_freeDDict(Dict);
        Dict = nullptr;
    }
}

bool P3A::Load(std::string_view path) {
    this->Clear();

    IO::File f(path, IO::OpenMode::Read);
    return Load(f);
}

bool P3A::Load(IO::File& f) {
    if (!f.IsOpen()) {
        return false;
    }

    P3AHeader header;
    if (f.Read(&header, sizeof(P3AHeader)) != sizeof(P3AHeader)) {
        return false;
    }
    const auto headerHash = XXH64(&header, sizeof(P3AHeader) - 8, 0);
    if (headerHash != header.Hash) {
        return false;
    }

    auto fileinfos = std::make_unique_for_overwrite<P3AFileInfo[]>(header.FileCount);
    if (!fileinfos) {
        return false;
    }
    const size_t fileinfoTotalSize = sizeof(P3AFileInfo) * header.FileCount;
    if (f.Read(fileinfos.get(), fileinfoTotalSize) != fileinfoTotalSize) {
        return false;
    }

    ZSTD_DDict_UniquePtr ddict = nullptr;
    if (header.Flags & P3AHeaderFlag_HasZstdDict) {
        P3ADictHeader dictHeader;
        if (f.Read(&dictHeader, sizeof(P3ADictHeader)) != sizeof(P3ADictHeader)) {
            return false;
        }
        auto dict = std::make_unique_for_overwrite<uint8_t[]>(dictHeader.Length);
        if (!dict) {
            return false;
        }
        if (f.Read(dict.get(), dictHeader.Length) != dictHeader.Length) {
            return false;
        }
        ddict = ZSTD_DDict_UniquePtr(ZSTD_createDDict(dict.get(), dictHeader.Length));
        if (!ddict) {
            return false;
        }
    }

    this->FileCount = header.FileCount;
    this->FileInfo = std::move(fileinfos);
    this->Dict = ddict.release();
    this->FileHandle = std::move(f);

    return true;
}
} // namespace SenPatcher
