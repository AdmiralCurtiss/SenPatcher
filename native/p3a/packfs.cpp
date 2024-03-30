#include "packfs.h"

#include <array>
#include <filesystem>
#include <vector>

#include "file.h"

#include "p3a/pack.h"
#include "p3a/structs.h"

namespace SenPatcher {
static bool CollectEntries(std::vector<P3APackFile>& fileinfos,
                           const std::filesystem::path& rootDir,
                           const std::filesystem::path& currentDir,
                           std::error_code& ec,
                           P3ACompressionType desiredCompressionType) {
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
            fn[i] = (c == '\\' ? '/' : c);
        }
        fileinfos.emplace_back(entry.path(), fn, desiredCompressionType);
    }
    return true;
}

bool PackP3AFromDirectory(const std::filesystem::path& directoryPath,
                          const std::filesystem::path& archivePath,
                          P3ACompressionType desiredCompressionType,
                          const std::filesystem::path& dictPath) {
    P3APackData packData;
    packData.Alignment = 0x40;
    std::error_code ec;
    if (!CollectEntries(packData.Files, directoryPath, directoryPath, ec, desiredCompressionType)) {
        return false;
    }

    // probably not needed but makes the packing order reproduceable
    std::stable_sort(packData.Files.begin(),
                     packData.Files.end(),
                     [](const P3APackFile& lhs, const P3APackFile& rhs) {
                         const auto& l = lhs.GetFilename();
                         const auto& r = rhs.GetFilename();
                         return memcmp(l.data(), r.data(), l.size()) < 0;
                     });

    if (!dictPath.empty()) {
        packData.ZStdDictionary = dictPath;
    }

    return PackP3A(archivePath, packData);
}

bool PackP3A(const std::filesystem::path& archivePath, const P3APackData& packData) {
    IO::File file(archivePath, IO::OpenMode::Write);
    return PackP3A(file, packData);
}
} // namespace SenPatcher
