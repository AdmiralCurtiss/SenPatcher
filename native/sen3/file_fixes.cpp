#include "file_fixes.h"

#include <optional>
#include <string_view>
#include <vector>

#include "../file.h"
#include "../file_getter.h"
#include "../p3a/pack.h"
#include "../sha1.h"

namespace SenLib::Sen3::FileFixes::a0417_dat {
std::string_view GetDescription();
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result);
} // namespace SenLib::Sen3::FileFixes::a0417_dat

namespace SenLib::Sen3 {
void CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, const std::filesystem::path& baseDir) {
    // TODO: check if the archive already exists and is at the correct version, and if yes don't
    // recreate it


    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const SenPatcher::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        SenPatcher::CheckedFileResult result{};
        if (path.size() > result.Filename.size()) {
            return std::nullopt;
        }
        std::memcpy(result.Filename.data(), path.data(), path.size());

        SenPatcher::IO::File inputfile(
            baseDir
                / std::u8string_view((const char8_t*)path.data(),
                                     (const char8_t*)(path.data() + path.size())),
            SenPatcher::IO::OpenMode::Read);
        if (!inputfile.IsOpen()) {
            return std::nullopt;
        }
        const auto filesize = inputfile.GetLength();
        if (!filesize || size != *filesize) {
            return std::nullopt;
        }
        std::vector<char>& vec = result.Data;
        vec.resize(size);
        if (inputfile.Read(vec.data(), size) != size) {
            return std::nullopt;
        }
        inputfile.Close();

        if (SenPatcher::CalculateSHA1(vec.data(), size) != hash) {
            return std::nullopt;
        }

        return result;
    };
    SenPatcher::P3APackData packData;
    packData.Alignment = 0x10;
    SenLib::Sen3::FileFixes::a0417_dat::TryApply(callback, packData.Files);
    SenPatcher::PackP3A(baseDir / L"mods/zzz_senpatcher_cs3.p3a", packData);
}
} // namespace SenLib::Sen3
