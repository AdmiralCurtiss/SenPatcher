#include "packjson.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "p3a/structs.h"

#include "file.h"

#include "rapidjson/document.h"

namespace SenPatcher {
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

static bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const char cl = (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i];
        const char cr = (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i];
        if (cl != cr) {
            return false;
        }
    }
    return true;
}

bool PackP3AFromJsonFile(const std::filesystem::path& jsonPath,
                         const std::filesystem::path& archivePath) {
    P3APackData packData;

    SenPatcher::IO::File f(jsonPath, SenPatcher::IO::OpenMode::Read);
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
    packData.Alignment = ReadUInt64(root, "Alignment").value_or(0);
    const auto dictString = ReadString(root, "ZStdDictionaryPath");
    packData.ZStdDictionary = std::monostate();
    if (dictString) {
        packData.ZStdDictionary = jsonPath.parent_path().append(
            std::u8string_view((const char8_t*)dictString->data(),
                               ((const char8_t*)dictString->data()) + dictString->size()));
    }
    packData.Files.clear();
    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                const auto fileobj = file.GetObject();
                const auto nameInArchive = ReadString(file, "NameInArchive");
                const auto pathOnDisk = ReadString(file, "PathOnDisk");
                const auto compression = ReadString(file, "Compression");
                if (!nameInArchive || !pathOnDisk || !compression) {
                    return false;
                }
                auto& pf = packData.Files.emplace_back();
                pf.Data = jsonPath.parent_path().append(
                    std::u8string_view((const char8_t*)pathOnDisk->data(),
                                       ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size()));
                pf.Filename.fill(0);
                if (nameInArchive->size() > pf.Filename.size()) {
                    return false;
                }
                std::memcpy(pf.Filename.data(), nameInArchive->data(), nameInArchive->size());

                if (CaseInsensitiveEquals(*compression, "none")) {
                    pf.DesiredCompressionType = SenPatcher::P3ACompressionType::None;
                } else if (CaseInsensitiveEquals(*compression, "lz4")) {
                    pf.DesiredCompressionType = SenPatcher::P3ACompressionType::LZ4;
                } else if (CaseInsensitiveEquals(*compression, "zStd")) {
                    pf.DesiredCompressionType = SenPatcher::P3ACompressionType::ZSTD;
                } else if (CaseInsensitiveEquals(*compression, "zStdDict")) {
                    pf.DesiredCompressionType = SenPatcher::P3ACompressionType::ZSTD_DICT;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    return PackP3A(archivePath, packData);
}
} // namespace SenPatcher
