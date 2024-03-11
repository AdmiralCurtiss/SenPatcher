#include "file_fixes.h"

#include <algorithm>
#include <cstring>
#include <optional>
#include <string_view>
#include <vector>

#include "../file.h"
#include "../file_getter.h"
#include "../p3a/p3a.h"
#include "../p3a/pack.h"
#include "../p3a/structs.h"
#include "../p3a/util.h"
#include "../sha1.h"

#include "senpatcher_version.h"

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen3::FileFixes::##name {                                 \
        std::string_view GetDescription();                                      \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

DECLARE_STANDARD_FIX(alchr022_dat)
DECLARE_STANDARD_FIX(a0417_dat)
DECLARE_STANDARD_FIX(book05_dat)
DECLARE_STANDARD_FIX(book06_dat)
DECLARE_STANDARD_FIX(book07_dat)
DECLARE_STANDARD_FIX(c0200_dat)
DECLARE_STANDARD_FIX(c0250_dat)
DECLARE_STANDARD_FIX(c0400_dat)
DECLARE_STANDARD_FIX(c0420_dat)
DECLARE_STANDARD_FIX(c0430_dat)
DECLARE_STANDARD_FIX(c0820_dat)
DECLARE_STANDARD_FIX(c0830_dat)
DECLARE_STANDARD_FIX(c2430_dat)
DECLARE_STANDARD_FIX(c2440_dat)
DECLARE_STANDARD_FIX(c2610_dat)
DECLARE_STANDARD_FIX(c3000_dat)
DECLARE_STANDARD_FIX(c3010_dat)
DECLARE_STANDARD_FIX(c3210_dat)
DECLARE_STANDARD_FIX(c3610_dat)
DECLARE_STANDARD_FIX(f0010_dat)
DECLARE_STANDARD_FIX(f2000_dat)
DECLARE_STANDARD_FIX(I_CVIS0061_pkg)
DECLARE_STANDARD_FIX(I_CVIS1008_pkg)
DECLARE_STANDARD_FIX(I_JMP009_pkg)
DECLARE_STANDARD_FIX(insa05)
DECLARE_STANDARD_FIX(insa08)
DECLARE_STANDARD_FIX(insa09)
DECLARE_STANDARD_FIX(m0000_dat)
DECLARE_STANDARD_FIX(m0100_dat)
DECLARE_STANDARD_FIX(m0300_dat)
DECLARE_STANDARD_FIX(m0600_dat)
DECLARE_STANDARD_FIX(m3000_dat)
DECLARE_STANDARD_FIX(m3420_dat)
DECLARE_STANDARD_FIX(m3430_dat)
DECLARE_STANDARD_FIX(m4004_dat)
DECLARE_STANDARD_FIX(r0210_dat)
DECLARE_STANDARD_FIX(r2290_dat)
DECLARE_STANDARD_FIX(r3000_dat)
DECLARE_STANDARD_FIX(r3200_dat)
DECLARE_STANDARD_FIX(r3430_dat)
DECLARE_STANDARD_FIX(r4200_dat)
DECLARE_STANDARD_FIX(r4290_dat)
DECLARE_STANDARD_FIX(t_item)
DECLARE_STANDARD_FIX(t_jump)
DECLARE_STANDARD_FIX(t_magic)
DECLARE_STANDARD_FIX(t_mons)
DECLARE_STANDARD_FIX(t_mstqrt)
DECLARE_STANDARD_FIX(t_name)
DECLARE_STANDARD_FIX(t_notecook)
DECLARE_STANDARD_FIX(t_place)
DECLARE_STANDARD_FIX(t_vctiming_us)
DECLARE_STANDARD_FIX(t0000_dat)
DECLARE_STANDARD_FIX(t0010_dat)
DECLARE_STANDARD_FIX(t0080_dat)
DECLARE_STANDARD_FIX(t0100_dat)
DECLARE_STANDARD_FIX(t0200_dat)
DECLARE_STANDARD_FIX(t0210_dat)
DECLARE_STANDARD_FIX(t0250_dat)
DECLARE_STANDARD_FIX(t0260_dat)
DECLARE_STANDARD_FIX(t0400_dat)
DECLARE_STANDARD_FIX(t0410_dat)
DECLARE_STANDARD_FIX(t3000_dat)
DECLARE_STANDARD_FIX(t3200_dat)
DECLARE_STANDARD_FIX(t3220_dat)
DECLARE_STANDARD_FIX(t3400_dat)
DECLARE_STANDARD_FIX(t3510_dat)
DECLARE_STANDARD_FIX(t3600_dat)
DECLARE_STANDARD_FIX(tk_ada_dat)
DECLARE_STANDARD_FIX(tk_linde_dat)
DECLARE_STANDARD_FIX(tk_patrick_dat)
DECLARE_STANDARD_FIX(tk_stark_dat)
DECLARE_STANDARD_FIX(tk_tovar_dat)
DECLARE_STANDARD_FIX(tk_zessica_dat)
DECLARE_STANDARD_FIX(v0010_dat)
DECLARE_STANDARD_FIX(v0050_dat)
DECLARE_STANDARD_FIX(voice_opus_ps4_103)
DECLARE_STANDARD_FIX(voice_opus_v00e0441)
DECLARE_STANDARD_FIX(voice_opus_v00s2728)

namespace SenLib::Sen3::FileFixes::t_text {
std::string_view GetDescription();
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              bool allowSwitchToNightmare);
} // namespace SenLib::Sen3::FileFixes::t_text

#define EARLY_EXIT(expr)  \
    do {                  \
        if (!expr)        \
            return false; \
    } while (false)

namespace SenLib::Sen3 {
static bool CollectAssets(const SenPatcher::GetCheckedFileCallback& callback,
                          SenPatcher::P3APackData& packData,
                          bool allowSwitchToNightmare) {
    EARLY_EXIT(SenLib::Sen3::FileFixes::a0417_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::alchr022_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::book05_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::book06_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::book07_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0200_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0250_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0400_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0420_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0430_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0820_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c0830_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c2430_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c2440_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c2610_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c3000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c3010_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c3210_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::c3610_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::f0010_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::f2000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::I_CVIS0061_pkg::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::I_CVIS1008_pkg::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::I_JMP009_pkg::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m0000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m0100_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m0300_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m0600_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m3000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m3420_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m3430_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::m4004_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r0210_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r2290_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r3000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r3200_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r3430_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r4200_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::r4290_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_item::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_jump::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_magic::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_mons::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_mstqrt::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_name::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_notecook::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_place::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_text::TryApply(
        callback, packData.Files, allowSwitchToNightmare));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t_vctiming_us::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0010_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0080_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0100_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0200_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0210_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0250_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0260_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0400_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t0410_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3000_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3200_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3220_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3400_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3510_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::t3600_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_ada_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_linde_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_patrick_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_stark_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_tovar_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::tk_zessica_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::v0010_dat::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::v0050_dat::TryApply(callback, packData.Files));
    return true;
}

static bool CollectAudio(const SenPatcher::GetCheckedFileCallback& callback,
                         SenPatcher::P3APackData& packData) {
    EARLY_EXIT(SenLib::Sen3::FileFixes::voice_opus_ps4_103::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::voice_opus_v00e0441::TryApply(callback, packData.Files));
    EARLY_EXIT(SenLib::Sen3::FileFixes::voice_opus_v00s2728::TryApply(callback, packData.Files));
    return true;
}

static bool IsSenPatcherVersionFile(const SenPatcher::P3AFileInfo& f) {
    return strcmp((const char*)f.Filename.data(), "_senpatcher_version.txt") == 0;
}

static bool CheckArchiveExistsAndIsRightVersion(const std::filesystem::path& path) {
    SenPatcher::P3A p3a;
    if (p3a.Load(path)) {
        for (size_t i = 0; i < p3a.FileCount; ++i) {
            const auto& f = p3a.FileInfo[i];
            if (IsSenPatcherVersionFile(f)) {
                std::array<char, sizeof(SENPATCHER_VERSION) - 1> versionInFile;
                if (f.CompressionType == SenPatcher::P3ACompressionType::None
                    && f.CompressedSize == versionInFile.size()
                    && p3a.FileHandle.SetPosition(f.Offset)
                    && p3a.FileHandle.Read(versionInFile.data(), versionInFile.size())
                           == versionInFile.size()) {
                    return std::string_view(SENPATCHER_VERSION)
                           == std::string_view(versionInFile.begin(), versionInFile.end());
                }
                return false;
            }
        }
    }
    return false;
}

static void AddSenPatcherVersionFile(SenPatcher::P3APackData& packData) {
    std::string_view sv(SENPATCHER_VERSION);
    std::vector<char> bin(sv.begin(), sv.end());
    packData.Files.emplace_back(
        SenPatcher::P3APackFile{std::move(bin),
                                SenPatcher::InitializeP3AFilename("_senpatcher_version.txt"),
                                SenPatcher::P3ACompressionType::None});
}

void CreateAssetPatchIfNeeded(SenPatcher::Logger& logger, const std::filesystem::path& baseDir) {
    // TODO: handle this flag somehow?
    bool allowSwitchToNightmare = true;


    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const SenPatcher::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        SenPatcher::CheckedFileResult result{};
        if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
            return std::nullopt;
        }

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
            // printf("filesize wrong -> %d == %s\n", (int)filesize.value_or(0), path.data());
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

    // check if the archives already exist and are at the correct version, and if yes don't recreate
    // them
    const auto assetArchivePath = baseDir / L"mods/zzz_senpatcher_cs3asset.p3a";
    const auto audioArchivePath = baseDir / L"mods/zzz_senpatcher_cs3audio.p3a";
    const auto videoArchivePath = baseDir / L"mods/zzz_senpatcher_cs3video.p3a";

    if (!CheckArchiveExistsAndIsRightVersion(assetArchivePath)) {
        SenPatcher::P3APackData packData;
        packData.Alignment = 0x10;
        AddSenPatcherVersionFile(packData);
        if (CollectAssets(callback, packData, allowSwitchToNightmare)) {
            std::stable_sort(
                packData.Files.begin(),
                packData.Files.end(),
                [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                    return memcmp(lhs.Filename.data(), rhs.Filename.data(), lhs.Filename.size())
                           < 0;
                });
            SenPatcher::PackP3A(assetArchivePath, packData);
        }
    }

    if (!CheckArchiveExistsAndIsRightVersion(audioArchivePath)) {
        SenPatcher::P3APackData packData;
        packData.Alignment = 0x10;
        AddSenPatcherVersionFile(packData);
        if (CollectAudio(callback, packData)) {
            std::stable_sort(
                packData.Files.begin(),
                packData.Files.end(),
                [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                    return memcmp(lhs.Filename.data(), rhs.Filename.data(), lhs.Filename.size())
                           < 0;
                });
            SenPatcher::PackP3A(audioArchivePath, packData);
        }
    }

    if (!CheckArchiveExistsAndIsRightVersion(videoArchivePath)) {
        SenPatcher::P3APackData packData;
        packData.Alignment = 0x10;
        AddSenPatcherVersionFile(packData);
        if (SenLib::Sen3::FileFixes::insa05::TryApply(callback, packData.Files)
            && SenLib::Sen3::FileFixes::insa08::TryApply(callback, packData.Files)
            && SenLib::Sen3::FileFixes::insa09::TryApply(callback, packData.Files)) {
            std::stable_sort(
                packData.Files.begin(),
                packData.Files.end(),
                [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                    return memcmp(lhs.Filename.data(), rhs.Filename.data(), lhs.Filename.size())
                           < 0;
                });
            SenPatcher::PackP3A(videoArchivePath, packData);
        }
    }
}
} // namespace SenLib::Sen3
