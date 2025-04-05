#include "pkg_repack.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "pkg_repack_main.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/sha256.h"

namespace SenTools {
template<typename T>
static std::optional<uint32_t> JsonReadUInt32(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsUint()) {
        const auto i = j.GetUint();
        return static_cast<uint32_t>(i);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<std::string> JsonReadString(T& json, const char* key) {
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

template<typename T>
static std::optional<bool> JsonReadBool(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsBool()) {
        return j.GetBool();
    }
    return std::nullopt;
}

int PKG_Repack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        PKG_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, extract with the -j option, then point this program at the __pkg.json that "
        "was generated during the archive extraction. You can also modify this file for some "
        "advanced packing features that are not available through the standard directory packing "
        "interface.");

    parser.usage("sentools " PKG_Repack_Name " [options] __pkg.json");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    auto* output_option = options.get("output");
    if (output_option == nullptr) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(output_option->first_string());

    auto result = RepackPkg(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<RepackPkgResult, std::string> RepackPkg(std::string_view source,
                                                            std::string_view target) {
    std::filesystem::path sourcepath = HyoutaUtils::IO::FilesystemPathFromUtf8(source);
    HyoutaUtils::IO::File f(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return std::format("Failed to open {}", source);
    }
    auto length = f.GetLength();
    if (!length) {
        return std::format("Failed to get length of {}", source);
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return std::format("Failed to allocate memory for {}", source);
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return std::format("Failed to read {}", source);
    }

    rapidjson::Document json;
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag
                   | rapidjson::kParseCommentsFlag,
               rapidjson::UTF8<char>>(buffer.get(), *length);
    if (json.HasParseError()) {
        return std::format("Failed to parse JSON: {}",
                           rapidjson::GetParseError_En(json.GetParseError()));
    }
    if (!json.IsObject()) {
        return std::string("JSON root is not an object");
    }

    const auto root = json.GetObject();

    std::vector<SenLib::PkgFile> fileinfos;
    std::vector<std::unique_ptr<char[]>> filedatas;
    const uint32_t unknownValue = JsonReadUInt32(root, "Unknown").value_or(0);

    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                const auto fileobj = file.GetObject();
                const auto nameInArchive = JsonReadString(file, "NameInArchive");
                if (!nameInArchive) {
                    return std::string("JSON error: 'NameInArchive' missing or invalid");
                }
                const auto pathOnDisk = JsonReadString(file, "PathOnDisk");
                if (!pathOnDisk) {
                    return std::string("JSON error: 'PathOnDisk' missing or invalid");
                }
                const auto flags = JsonReadUInt32(file, "Flags");
                if (!flags) {
                    return std::string("JSON error: 'Flags' missing or invalid");
                }
                const auto isPkaRef = JsonReadBool(file, "IsPkaReference");
                std::array<char, 0x40> fn{};
                if (nameInArchive->size() > fn.size()) {
                    return std::format("JSON error: 'NameInArchive' too long ({})", *nameInArchive);
                }
                std::memcpy(fn.data(), nameInArchive->data(), nameInArchive->size());

                auto& fi = fileinfos.emplace_back(SenLib::PkgFile{.Filename = fn});
                auto& fd = filedatas.emplace_back();

                HyoutaUtils::IO::File infile(
                    sourcepath.parent_path().append(std::u8string_view(
                        (const char8_t*)pathOnDisk->data(),
                        ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size())),
                    HyoutaUtils::IO::OpenMode::Read);
                if (!infile.IsOpen()) {
                    return std::format("Failed opening {}", *pathOnDisk);
                }
                const auto uncompressedLength = infile.GetLength();
                if (!uncompressedLength) {
                    return std::format("Failed getting size of {}", *pathOnDisk);
                }
                if (*uncompressedLength > UINT32_MAX) {
                    return std::format("{} too big to put into pkg", *pathOnDisk);
                }
                auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
                if (infile.Read(uncompressedData.get(), *uncompressedLength)
                    != *uncompressedLength) {
                    return std::format("Failed to read {}", *pathOnDisk);
                }

                if (isPkaRef.has_value() && *isPkaRef) {
                    auto hash = HyoutaUtils::Hash::CalculateSHA256(uncompressedData.get(),
                                                                   *uncompressedLength);
                    fd = std::make_unique<char[]>(hash.Hash.size());
                    std::memcpy(fd.get(), hash.Hash.data(), hash.Hash.size());
                    fi.UncompressedSize = static_cast<uint32_t>(*uncompressedLength);
                    fi.CompressedSize = static_cast<uint32_t>(hash.Hash.size());
                    fi.Flags = 0x80;
                    fi.Data = fd.get();
                } else {
                    if (!SenLib::CompressPkgFile(
                            fd,
                            fi,
                            uncompressedData.get(),
                            static_cast<uint32_t>(*uncompressedLength),
                            *flags,
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                        return std::string("Failed adding file to pkg");
                    }
                }
            } else {
                return std::string("JSON error: File is not a JSON object");
            }
        }
    } else {
        return std::string("JSON error: 'Files' not found or not an array");
    }

    std::unique_ptr<char[]> ms;
    size_t msSize;
    if (!SenLib::CreatePkgInMemory(ms,
                                   msSize,
                                   fileinfos.data(),
                                   static_cast<uint32_t>(fileinfos.size()),
                                   unknownValue,
                                   HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        return std::string("Failed to create pkg");
    }

    HyoutaUtils::IO::File outfile(target, HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        return std::string("Failed to open output file");
    }

    if (outfile.Write(ms.get(), msSize) != msSize) {
        return std::string("Failed to write to output file");
    }

    return RepackPkgResult::Success;
}
} // namespace SenTools
