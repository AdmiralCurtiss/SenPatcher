#include "unpack.h"

#include <filesystem>
#include <memory>
#include <string_view>

#include "lz4/lz4.h"

#include "zstd/common/xxhash.h"
#include "zstd/zstd.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "p3a/structs.h"

#include "file.h"

namespace SenPatcher {
static std::string_view StripTrailingNull(std::string_view sv) {
    std::string_view s = sv;
    while (!s.empty() && s.back() == '\0') {
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

bool UnpackP3A(const std::filesystem::path& archivePath, const std::filesystem::path& extractPath) {
    rapidjson::StringBuffer jsonbuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
    json.StartObject();

    IO::File f(archivePath, IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(extractPath, ec);
    if (ec) {
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

    json.Key("Version");
    json.Uint(header.Version);

    json.Key("Alignment");
    json.Uint(0);

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

        IO::File f2(extractPath / L"__zstd_dictionary.bin", IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return false;
        }
        if (f2.Write(dict.get(), dictHeader.Length) != dictHeader.Length) {
            return false;
        }

        json.Key("ZStdDictionaryPath");
        json.String("__zstd_dictionary.bin");
    }

    json.Key("Files");
    json.StartArray();

    for (size_t i = 0; i < header.FileCount; ++i) {
        json.StartObject();

        const auto& fileinfo = fileinfos[i];
        auto filename =
            StripTrailingNull(std::string_view(fileinfo.Filename.begin(), fileinfo.Filename.end()));
        std::filesystem::path relativePath(
            std::u8string_view((const char8_t*)filename.data(), filename.size()));
        if (relativePath.is_absolute()) {
            return false; // there's probably a better way to handle this case...
        }

        {
            json.Key("NameInArchive");
            json.String(filename.data(), filename.size());

            auto relPathStr = relativePath.u8string();
            json.Key("PathOnDisk");
            json.String((const char*)relPathStr.data(), relPathStr.size());
        }

        std::filesystem::path fullPath = extractPath / relativePath;
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

        if (fileinfo.Hash != XXH64(filedata.get(), fileinfo.CompressedSize, 0)) {
            return false;
        }

        json.Key("Compression");
        if (fileinfo.CompressionType == P3ACompressionType::LZ4) {
            json.String("lz4");
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
            json.String("zStd");
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
            json.String("zStdDict");
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
            json.String("none");
            if (fileinfo.CompressedSize != fileinfo.UncompressedSize) {
                return false;
            }
        }

        if (f2.Write(filedata.get(), fileinfo.UncompressedSize) != fileinfo.UncompressedSize) {
            return false;
        }

        json.EndObject();
    }

    json.EndArray();
    json.EndObject();

    {
        IO::File f2(extractPath / L"__p3a.json", IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return false;
        }

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (f2.Write(jsonstring, jsonstringsize) != jsonstringsize) {
            return false;
        }
    }

    return true;
}
} // namespace SenPatcher
