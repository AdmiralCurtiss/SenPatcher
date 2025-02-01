#include "packfs.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "util/file.h"
#include "util/text.h"

#include "p3a/pack.h"
#include "p3a/structs.h"

namespace SenPatcher {
static P3ACompressionType
    DetermineCompressionTypeForFile(const std::filesystem::path& path,
                                    std::optional<P3ACompressionType> desiredCompressionType) {
    if (desiredCompressionType) {
        return *desiredCompressionType;
    }

    // sloppy autodetection of 'do not compress' filetypes based on extension,
    // this should give reasonable results for the set of known game files
    auto ext = path.extension();
    if (!ext.empty()) {
        auto utf8ext = ext.u8string();
        std::string_view sv((const char*)utf8ext.data(), utf8ext.size());
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".pkg", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".pka", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".p3a", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".ogg", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".opus", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".png", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".avi", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".webm", sv)) {
            return P3ACompressionType::None;
        }
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(".wmv", sv)) {
            return P3ACompressionType::None;
        }
    }

    // just use lz4 for everything else
    return P3ACompressionType::LZ4;
}

static bool CollectEntries(std::vector<P3APackFile>& fileinfos,
                           const std::filesystem::path& rootDir,
                           const std::filesystem::path& currentDir,
                           std::error_code& ec,
                           std::optional<P3ACompressionType> desiredCompressionType,
                           bool allowUppercaseInFilenames) {
    std::filesystem::directory_iterator iterator(currentDir, ec);
    if (ec) {
        return false;
    }
    while (iterator != std::filesystem::directory_iterator()) {
        auto& entryPath = iterator->path();
        if (iterator->is_directory()) {
            if (!CollectEntries(fileinfos,
                                rootDir,
                                entryPath,
                                ec,
                                desiredCompressionType,
                                allowUppercaseInFilenames)) {
                return false;
            }
            continue;
        } else {
            const auto relativePath = std::filesystem::relative(entryPath, rootDir, ec);
            if (relativePath.empty()) {
                return false;
            }
            const auto filename = relativePath.u8string();
            const size_t filenameLength = filename.size();
            if (filenameLength > 0x100) {
                // maximum length of p3a filename
                return false;
            }

            std::array<char, 0x100> fn{};
            for (size_t i = 0; i < filenameLength; ++i) {
                const char c = static_cast<char>(filename[i]);
                if (c == '\0') {
                    // mid-filename nulls are not supported, would truncate the rest of the name
                    return false;
                }
                fn[i] = c;
            }

            fileinfos.emplace_back(
                entryPath,
                NormalizeP3AFilename(fn, allowUppercaseInFilenames),
                DetermineCompressionTypeForFile(entryPath, desiredCompressionType));
        }
        iterator.increment(ec);
        if (ec) {
            return false;
        }
    }
    return true;
}

std::optional<SenPatcher::P3APackData>
    P3APackDataFromDirectory(const std::filesystem::path& directoryPath,
                             uint32_t archiveVersion,
                             std::optional<P3ACompressionType> desiredCompressionType,
                             const std::filesystem::path& dictPath,
                             bool allowUppercaseInFilenames) {
    P3APackData packData;
    packData.SetVersion(archiveVersion);
    packData.SetAlignment(0x40);
    std::error_code ec;
    auto& packFiles = packData.GetMutableFiles();
    if (!CollectEntries(packFiles,
                        directoryPath,
                        directoryPath,
                        ec,
                        desiredCompressionType,
                        allowUppercaseInFilenames)) {
        return std::nullopt;
    }

    // probably not needed but makes the packing order reproduceable
    std::stable_sort(
        packFiles.begin(), packFiles.end(), [](const P3APackFile& lhs, const P3APackFile& rhs) {
            const auto& lhsName = lhs.GetFilename();
            const auto& rhsName = rhs.GetFilename();
            return HyoutaUtils::TextUtils::CaseInsensitiveCompare(
                       std::string_view(lhsName.data(), lhsName.data() + lhsName.size()),
                       std::string_view(rhsName.data(), rhsName.data() + rhsName.size()))
                   < 0;
        });

    if (!dictPath.empty()) {
        packData.SetZStdDictionaryPathData(dictPath);
    }

    packData.SetAllowUppercaseInFilenames(allowUppercaseInFilenames);

    return packData;
}
} // namespace SenPatcher
