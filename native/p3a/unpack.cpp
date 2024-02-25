#include "unpack.h"

#include <filesystem>
#include <memory>
#include <string_view>

#include "../lz4/lz4.h"

#include "../zstd/common/xxhash.h"
#include "../zstd/zstd.h"

#include "structs.h"

#include "../file.h"

namespace SenPatcher {
template<typename CharT, typename Traits>
static std::basic_string_view<CharT, Traits>
    StripTrailingNull(std::basic_string_view<CharT, Traits> sv) {
    std::basic_string_view<CharT, Traits> s = sv;
    while (!s.empty() && s.back() == char8_t(0)) {
        s = s.substr(0, s.size() - 1);
    }
    return s;
}

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

bool UnpackP3A(const std::filesystem::path& archivePath,
               const std::filesystem::path& extractPath,
               const std::filesystem::path& extractPathZstdDict) {
    IO::File f(archivePath, IO::OpenMode::Read);
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

        if (!extractPathZstdDict.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(extractPathZstdDict.parent_path(), ec);
            if (ec) {
                return false;
            }
            IO::File f2(extractPathZstdDict, IO::OpenMode::Write);
            if (!f2.IsOpen()) {
                return false;
            }
            if (f2.Write(dict.get(), dictHeader.Length) != dictHeader.Length) {
                return false;
            }
        }
    }

    for (size_t i = 0; i < header.FileCount; ++i) {
        const auto& fileinfo = fileinfos[i];
        auto filename = StripTrailingNull(
            std::basic_string_view<char8_t>(fileinfo.Filename.begin(), fileinfo.Filename.end()));
        std::filesystem::path relativePath(filename.begin(), filename.end());
        if (relativePath.is_absolute()) {
            return false; // there's probably a better way to handle this case...
        }

        std::filesystem::path fullPath = extractPath / relativePath;
        std::error_code ec;
        std::filesystem::create_directories(fullPath.parent_path(), ec);
        if (ec) {
            return false;
        }
        IO::File f2(fullPath, IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return false;
        }

        // not particularly memory efficient but whatever
        auto filedata = std::make_unique_for_overwrite<char[]>(fileinfo.CompressedSize);
        if (!filedata) {
            return false;
        }
        if (!f.SetPosition(fileinfo.Offset)) {
            return false;
        }
        if (f.Read(filedata.get(), fileinfo.CompressedSize) != fileinfo.CompressedSize) {
            return false;
        }

        if (fileinfo.CompressionType == P3ACompressionType::LZ4) {
            auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
            if (!decomp) {
                return false;
            }
            if (LZ4_decompress_safe(filedata.get(),
                                    decomp.get(),
                                    fileinfo.CompressedSize,
                                    fileinfo.UncompressedSize)
                != fileinfo.UncompressedSize) {
                return false;
            }
            decomp.swap(filedata);
        } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD) {
            auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
            if (!decomp) {
                return false;
            }
            if (ZSTD_decompress(decomp.get(),
                                fileinfo.UncompressedSize,
                                filedata.get(),
                                fileinfo.CompressedSize)
                != fileinfo.UncompressedSize) {
                return false;
            }
            decomp.swap(filedata);
        } else if (fileinfo.CompressionType == P3ACompressionType::ZSTD_DICT) {
            if (!ddict) {
                return false;
            }

            auto decomp = std::make_unique_for_overwrite<char[]>(fileinfo.UncompressedSize);
            if (!decomp) {
                return false;
            }
            ZSTD_DCtx_UniquePtr dctx = ZSTD_DCtx_UniquePtr(ZSTD_createDCtx());
            if (!dctx) {
                return false;
            }
            if (ZSTD_decompress_usingDDict(dctx.get(),
                                           decomp.get(),
                                           fileinfo.UncompressedSize,
                                           filedata.get(),
                                           fileinfo.CompressedSize,
                                           ddict.get())
                != fileinfo.UncompressedSize) {
                return false;
            }

            decomp.swap(filedata);
        } else {
            if (fileinfo.CompressedSize != fileinfo.UncompressedSize) {
                return false;
            }
        }

        if (f2.Write(filedata.get(), fileinfo.UncompressedSize) != fileinfo.UncompressedSize) {
            return false;
        }
    }

    return true;
}
} // namespace SenPatcher
