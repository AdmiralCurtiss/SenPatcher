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
                           std::optional<P3ACompressionType> desiredCompressionType) {
    std::filesystem::directory_iterator iterator(currentDir, ec);
    if (ec) {
        return false;
    }
    for (auto const& entry : iterator) {
        if (entry.is_directory()) {
            if (!CollectEntries(fileinfos, rootDir, entry.path(), ec, desiredCompressionType)) {
                return false;
            }
            continue;
        }

        const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
        if (relativePath.empty()) {
            return false;
        }
        const auto filename = relativePath.u8string();
        const char8_t* filenameC = filename.c_str();

        std::array<char, 0x100> fn{};
        for (size_t i = 0; i < fn.size(); ++i) {
            const char c = static_cast<char>(filenameC[i]);
            if (c == '\0') {
                break;
            }
            fn[i] = c;
        }

        auto& entryPath = entry.path();
        fileinfos.emplace_back(entryPath,
                               NormalizeP3AFilename(fn),
                               DetermineCompressionTypeForFile(entryPath, desiredCompressionType));
    }
    return true;
}

bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          uint32_t archiveVersion,
                          std::optional<P3ACompressionType> desiredCompressionType,
                          const std::filesystem::path& dictPath,
                          size_t desiredThreadCount) {
    P3APackData packData;
    packData.SetVersion(archiveVersion);
    packData.SetAlignment(0x40);
    std::error_code ec;
    auto& packFiles = packData.GetMutableFiles();
    if (!CollectEntries(packFiles, directoryPath, directoryPath, ec, desiredCompressionType)) {
        return false;
    }

    // probably not needed but makes the packing order reproduceable
    std::stable_sort(
        packFiles.begin(), packFiles.end(), [](const P3APackFile& lhs, const P3APackFile& rhs) {
            const auto& l = lhs.GetFilename();
            const auto& r = rhs.GetFilename();
            return memcmp(l.data(), r.data(), l.size()) < 0;
        });

    if (!dictPath.empty()) {
        packData.SetZStdDictionaryPathData(dictPath);
    }

    return PackP3A(archivePath, packData, desiredThreadCount);
}

bool PackP3A(const std::filesystem::path& archivePath,
             const P3APackData& packData,
             size_t desiredThreadCount) {
    HyoutaUtils::IO::File file(archivePath, HyoutaUtils::IO::OpenMode::Write);
    return PackP3A(file, packData, desiredThreadCount);
}
} // namespace SenPatcher
