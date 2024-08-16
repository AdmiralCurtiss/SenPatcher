#include "packjson.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "p3a/packfs.h"
#include "p3a/structs.h"
#include "util/text.h"

#include "util/file.h"

#include "rapidjson/document.h"

namespace SenPatcher {
template<typename T>
static std::optional<bool> ReadBool(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsBool()) {
        return j.GetBool() ? true : false;
    }
    return std::nullopt;
}

template<typename T>
static std::optional<uint64_t> ReadUInt64(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsInt64()) {
        const auto i = j.GetInt64();
        if (i < 0) {
            return std::nullopt;
        }
        return static_cast<uint64_t>(i);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<std::string> ReadString(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsString()) {
        const char* str = j.GetString();
        const auto len = j.GetStringLength();
        return std::string(str, len);
    }
    return std::nullopt;
}

bool PackP3AFromJsonFile(const std::filesystem::path& jsonPath,
                         const std::filesystem::path& archivePath,
                         size_t desiredThreadCount) {
    P3APackData packData;

    HyoutaUtils::IO::File f(jsonPath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return false;
    }
    auto length = f.GetLength();
    if (!length) {
        return false;
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return false;
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return false;
    }

    rapidjson::Document json;
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag,
               rapidjson::UTF8<char>>(buffer.get(), *length);
    if (json.HasParseError() || !json.IsObject()) {
        return false;
    }

    const auto root = json.GetObject();
    packData.SetAlignment(ReadUInt64(root, "Alignment").value_or(0));
    const auto dictString = ReadString(root, "ZStdDictionaryPath");
    packData.ClearZStdDictionaryData();
    if (dictString) {
        packData.SetZStdDictionaryPathData(jsonPath.parent_path().append(
            std::u8string_view((const char8_t*)dictString->data(),
                               ((const char8_t*)dictString->data()) + dictString->size())));
    }
    auto& packDataFiles = packData.GetMutableFiles();
    packDataFiles.clear();
    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                const auto fileobj = file.GetObject();
                const auto nameInArchive = ReadString(file, "NameInArchive");
                const auto pathOnDisk = ReadString(file, "PathOnDisk");
                const auto compression = ReadString(file, "Compression");
                const bool isPrecompressed = ReadBool(file, "Precompressed").value_or(false);
                const auto precompressedUncompressedFilesize =
                    ReadUInt64(file, "UncompressedFilesize");
                if (!nameInArchive || !pathOnDisk || !compression) {
                    return false;
                }
                if (isPrecompressed && !precompressedUncompressedFilesize) {
                    return false;
                }
                std::array<char, 0x100> fn{};
                if (nameInArchive->size() > fn.size()) {
                    return false;
                }
                std::memcpy(fn.data(), nameInArchive->data(), nameInArchive->size());
                SenPatcher::P3ACompressionType ct = SenPatcher::P3ACompressionType::None;
                using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                if (CaseInsensitiveEquals(*compression, "none")) {
                    ct = SenPatcher::P3ACompressionType::None;
                } else if (CaseInsensitiveEquals(*compression, "lz4")) {
                    ct = SenPatcher::P3ACompressionType::LZ4;
                } else if (CaseInsensitiveEquals(*compression, "zStd")) {
                    ct = SenPatcher::P3ACompressionType::ZSTD;
                } else if (CaseInsensitiveEquals(*compression, "zStdDict")) {
                    ct = SenPatcher::P3ACompressionType::ZSTD_DICT;
                } else {
                    return false;
                }

                packDataFiles.emplace_back(
                    jsonPath.parent_path().append(std::u8string_view(
                        (const char8_t*)pathOnDisk->data(),
                        ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size())),
                    fn,
                    ct,
                    isPrecompressed ? P3APackFilePrecompressed::Yes : P3APackFilePrecompressed::No,
                    isPrecompressed ? *precompressedUncompressedFilesize : 0u);
            } else {
                return false;
            }
        }
    }

    return PackP3A(archivePath, packData, desiredThreadCount);
}
} // namespace SenPatcher
