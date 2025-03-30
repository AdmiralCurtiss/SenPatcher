#include "file_fixes.h"

#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "zstd/common/xxhash.h"

#include "modload/loaded_pka.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "util/logger.h"

#include "senpatcher_version.h"

static constexpr char Data_SenpatcherModIniDefault[] = "[CS3Mod]\nMinFeatureLevel=0";
static constexpr size_t Length_SenpatcherModIniDefault = sizeof(Data_SenpatcherModIniDefault) - 1;

#define DECLARE_STANDARD_FIX(name)                                              \
    namespace SenLib::Sen3::FileFixes::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }

#define DECLARE_PKA_FIX(name)                                                           \
    namespace SenLib::Sen3::FileFixes::##name {                                         \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,         \
                      std::vector<SenPatcher::P3APackFile>& result,                     \
                      SenLib::ModLoad::LoadedPkaData& vanillaPKAs,                      \
                      const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub); \
    }

DECLARE_STANDARD_FIX(alchr022_dat)
DECLARE_STANDARD_FIX(a0417_dat)
DECLARE_STANDARD_FIX(book05_dat)
DECLARE_STANDARD_FIX(book06_dat)
DECLARE_STANDARD_FIX(book07_dat)
DECLARE_STANDARD_FIX(c0000_dat)
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
DECLARE_STANDARD_FIX(r3090_dat)
DECLARE_STANDARD_FIX(r3200_dat)
DECLARE_STANDARD_FIX(r3430_dat)
DECLARE_STANDARD_FIX(r4200_dat)
DECLARE_STANDARD_FIX(r4290_dat)
DECLARE_STANDARD_FIX(t_item_magic)
DECLARE_STANDARD_FIX(t_itemhelp)
DECLARE_STANDARD_FIX(t_jump)
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
DECLARE_STANDARD_FIX(v0030_dat)
DECLARE_STANDARD_FIX(v0050_dat)
DECLARE_STANDARD_FIX(voice_opus_ps4_103)
DECLARE_STANDARD_FIX(voice_opus_v00e0441)
DECLARE_STANDARD_FIX(voice_opus_v00s2728)

DECLARE_PKA_FIX(M_C0000_pkg)
DECLARE_PKA_FIX(M_C0200_pkg)
DECLARE_PKA_FIX(M_C2020_pkg)
DECLARE_PKA_FIX(M_C2600_O_C26HOU04_O_C26OBJ04_pkg)
DECLARE_PKA_FIX(M_T0010_M_A2003_O_T00HOU01_pkg)
DECLARE_PKA_FIX(M_T0020_M_C2010_pkg)
DECLARE_PKA_FIX(M_T0030_M_T1020_M_T2010_M_T3410_pkg)
DECLARE_PKA_FIX(M_T0210_M_T0230_M_T0240_M_T0250_M_T0260_pkg)
DECLARE_PKA_FIX(M_T1000_pkg)
DECLARE_PKA_FIX(M_T1200_M_T1220_pkg)
DECLARE_PKA_FIX(O_C02OBJ04_pkg)
DECLARE_PKA_FIX(O_C04OBJ01_pkg)
DECLARE_PKA_FIX(O_C08OBJ06_pkg)
DECLARE_PKA_FIX(O_R00OBJ10_O_R00OBJ11_O_R00OBJ12_O_R00OBJ13_pkg)
DECLARE_PKA_FIX(O_R12DOR00_pkg)
DECLARE_PKA_FIX(O_R14OBJ01_pkg)
DECLARE_PKA_FIX(O_R30OBJ10_O_R30OBJ11_pkg)
DECLARE_PKA_FIX(O_R42OBJ10_O_R42OBJ11_pkg)
DECLARE_PKA_FIX(O_S50OBJ11_pkg)
DECLARE_PKA_FIX(O_T02EVT16_pkg)
DECLARE_PKA_FIX(O_T02OBJ07_pkg)
DECLARE_PKA_FIX(O_T10OBJ02_pkg)
DECLARE_PKA_FIX(O_T10OBJ05_pkg)
DECLARE_PKA_FIX(O_T40OBJ01_pkg)
DECLARE_PKA_FIX(O_V80TNK00_pkg)

namespace SenLib::Sen3::FileFixes::t_text {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result,
              bool allowSwitchToNightmare);
} // namespace SenLib::Sen3::FileFixes::t_text

namespace SenLib::Sen3::FileFixes::insa05 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result);
} // namespace SenLib::Sen3::FileFixes::insa05
namespace SenLib::Sen3::FileFixes::insa08 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result);
} // namespace SenLib::Sen3::FileFixes::insa08
namespace SenLib::Sen3::FileFixes::insa09 {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, std::vector<char>& result);
} // namespace SenLib::Sen3::FileFixes::insa09

#define TRY_APPLY(asset, apply)                         \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        if (!(SenLib::Sen3::FileFixes::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
        logger.Log("\n");                               \
    } while (false)

namespace SenLib::Sen3 {
static bool CollectAssets(HyoutaUtils::Logger& logger,
                          const SenPatcher::GetCheckedFileCallback& callback,
                          std::vector<SenPatcher::P3APackFile>& packFiles,
                          bool allowSwitchToNightmare) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(a0417_dat, TryApply(callback, packFiles));
    TRY_APPLY(alchr022_dat, TryApply(callback, packFiles));
    TRY_APPLY(book05_dat, TryApply(callback, packFiles));
    TRY_APPLY(book06_dat, TryApply(callback, packFiles));
    TRY_APPLY(book07_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0000_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0200_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0250_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0400_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0420_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0430_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0820_dat, TryApply(callback, packFiles));
    TRY_APPLY(c0830_dat, TryApply(callback, packFiles));
    TRY_APPLY(c2430_dat, TryApply(callback, packFiles));
    TRY_APPLY(c2440_dat, TryApply(callback, packFiles));
    TRY_APPLY(c2610_dat, TryApply(callback, packFiles));
    TRY_APPLY(c3000_dat, TryApply(callback, packFiles));
    TRY_APPLY(c3010_dat, TryApply(callback, packFiles));
    TRY_APPLY(c3210_dat, TryApply(callback, packFiles));
    TRY_APPLY(c3610_dat, TryApply(callback, packFiles));
    TRY_APPLY(f0010_dat, TryApply(callback, packFiles));
    TRY_APPLY(f2000_dat, TryApply(callback, packFiles));
    TRY_APPLY(I_CVIS0061_pkg, TryApply(callback, packFiles));
    TRY_APPLY(I_CVIS1008_pkg, TryApply(callback, packFiles));
    TRY_APPLY(I_JMP009_pkg, TryApply(callback, packFiles));
    TRY_APPLY(m0000_dat, TryApply(callback, packFiles));
    TRY_APPLY(m0100_dat, TryApply(callback, packFiles));
    TRY_APPLY(m0300_dat, TryApply(callback, packFiles));
    TRY_APPLY(m0600_dat, TryApply(callback, packFiles));
    TRY_APPLY(m3000_dat, TryApply(callback, packFiles));
    TRY_APPLY(m3420_dat, TryApply(callback, packFiles));
    TRY_APPLY(m3430_dat, TryApply(callback, packFiles));
    TRY_APPLY(m4004_dat, TryApply(callback, packFiles));
    TRY_APPLY(r0210_dat, TryApply(callback, packFiles));
    TRY_APPLY(r2290_dat, TryApply(callback, packFiles));
    TRY_APPLY(r3000_dat, TryApply(callback, packFiles));
    TRY_APPLY(r3090_dat, TryApply(callback, packFiles));
    TRY_APPLY(r3200_dat, TryApply(callback, packFiles));
    TRY_APPLY(r3430_dat, TryApply(callback, packFiles));
    TRY_APPLY(r4200_dat, TryApply(callback, packFiles));
    TRY_APPLY(r4290_dat, TryApply(callback, packFiles));
    TRY_APPLY(t_item_magic, TryApply(callback, packFiles));
    TRY_APPLY(t_itemhelp, TryApply(callback, packFiles));
    TRY_APPLY(t_jump, TryApply(callback, packFiles));
    TRY_APPLY(t_mons, TryApply(callback, packFiles));
    TRY_APPLY(t_mstqrt, TryApply(callback, packFiles));
    TRY_APPLY(t_name, TryApply(callback, packFiles));
    TRY_APPLY(t_notecook, TryApply(callback, packFiles));
    TRY_APPLY(t_place, TryApply(callback, packFiles));
    TRY_APPLY(t_text, TryApply(callback, packFiles, allowSwitchToNightmare));
    TRY_APPLY(t_vctiming_us, TryApply(callback, packFiles));
    TRY_APPLY(t0000_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0010_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0080_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0100_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0200_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0210_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0250_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0260_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0400_dat, TryApply(callback, packFiles));
    TRY_APPLY(t0410_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3000_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3200_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3220_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3400_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3510_dat, TryApply(callback, packFiles));
    TRY_APPLY(t3600_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_ada_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_linde_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_patrick_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_stark_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_tovar_dat, TryApply(callback, packFiles));
    TRY_APPLY(tk_zessica_dat, TryApply(callback, packFiles));
    TRY_APPLY(v0010_dat, TryApply(callback, packFiles));
    TRY_APPLY(v0030_dat, TryApply(callback, packFiles));
    TRY_APPLY(v0050_dat, TryApply(callback, packFiles));
    return true;
}

static bool CollectAudio(HyoutaUtils::Logger& logger,
                         const SenPatcher::GetCheckedFileCallback& callback,
                         std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniDefault,
                          Data_SenpatcherModIniDefault + Length_SenpatcherModIniDefault),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    TRY_APPLY(voice_opus_ps4_103, TryApply(callback, packFiles));
    TRY_APPLY(voice_opus_v00e0441, TryApply(callback, packFiles));
    TRY_APPLY(voice_opus_v00s2728, TryApply(callback, packFiles));
    return true;
}

// This is a workaround. Older SenPatcher builds cannot handle the 'stub' pkg files that reference
// pka files via hash, but will attempt to load them anyway, which causes the game to misbehave. By
// storing the files at a 'wrong' path in the p3a but remapping the path on load, only SenPatcher
// versions that already support this feature will be able to correctly load the file.
static constexpr char Data_SenpatcherModIniPkaRemap[] =
    "[CS3Mod]\nMinFeatureLevel=1\nStripPathPrefix=pka/";
static constexpr size_t Length_SenpatcherModIniPkaRemap = sizeof(Data_SenpatcherModIniPkaRemap) - 1;

static bool CollectPkaAssets(HyoutaUtils::Logger& logger,
                             const SenPatcher::GetCheckedFileCallback& callback,
                             std::vector<SenPatcher::P3APackFile>& packFiles,
                             SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                             const SenPatcher::GetCheckedFileCallback& callbackPkaStub) {
    TRY_APPLY(M_C0000_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_C0200_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_C2020_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_C2600_O_C26HOU04_O_C26OBJ04_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T0010_M_A2003_O_T00HOU01_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T0020_M_C2010_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T0030_M_T1020_M_T2010_M_T3410_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T0210_M_T0230_M_T0240_M_T0250_M_T0260_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T1000_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(M_T1200_M_T1220_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_C02OBJ04_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_C04OBJ01_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_C08OBJ06_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_R00OBJ10_O_R00OBJ11_O_R00OBJ12_O_R00OBJ13_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_R12DOR00_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_R14OBJ01_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_R30OBJ10_O_R30OBJ11_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_R42OBJ10_O_R42OBJ11_pkg,
              TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_S50OBJ11_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_T02EVT16_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_T02OBJ07_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_T10OBJ02_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_T10OBJ05_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_T40OBJ01_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));
    TRY_APPLY(O_V80TNK00_pkg, TryApply(callback, packFiles, vanillaPKAs, callbackPkaStub));

    for (auto& f : packFiles) {
        std::array<char, 256> path = f.GetFilename();
        for (size_t i = path.size() - 1; i >= 4; --i) {
            path[i] = path[i - 4];
        }
        path[0] = 'p';
        path[1] = 'k';
        path[2] = 'a';
        path[3] = '/';
        f.SetFilename(path);
    }

    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniPkaRemap,
                          Data_SenpatcherModIniPkaRemap + Length_SenpatcherModIniPkaRemap),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);
    return true;
}

bool CreateAssetPatchIfNeeded(HyoutaUtils::Logger& logger,
                              std::string_view baseDir,
                              SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                              SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                              std::span<const std::string_view> pkaPrefixes,
                              SenLib::ModLoad::LoadedPkaGroupData& pkgsOfPrefix0File0,
                              bool allowSwitchToNightmare) {
    std::string_view versionString(SENPATCHER_VERSION, sizeof(SENPATCHER_VERSION) - 1);
    std::string versionStringWithSettings =
        std::format("{}:{}", versionString, allowSwitchToNightmare ? 1 : 0);
    std::string versionStringWithPka =
        std::format("{}:{}/{:x}/{}/{:x}",
                    versionString,
                    pkgsOfPrefix0File0.PkgCount,
                    XXH64(pkgsOfPrefix0File0.Pkgs.get(),
                          pkgsOfPrefix0File0.PkgCount * sizeof(SenLib::PkaPkgToHashData),
                          0),
                    pkgsOfPrefix0File0.PkgFileCount,
                    XXH64(pkgsOfPrefix0File0.PkgFiles.get(),
                          pkgsOfPrefix0File0.PkgFileCount * sizeof(SenLib::PkaFileHashData),
                          0));

    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(baseDir,
                              vanillaP3As,
                              vanillaPKAs,
                              pkaPrefixes,
                              &pkgsOfPrefix0File0,
                              path,
                              size,
                              hash,
                              false);
    };
    const SenPatcher::GetCheckedFileCallback callbackPkaStub =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        return GetCheckedFile(baseDir,
                              vanillaP3As,
                              vanillaPKAs,
                              pkaPrefixes,
                              &pkgsOfPrefix0File0,
                              path,
                              size,
                              hash,
                              true);
    };

    bool success = true;
    success = CreateArchiveIfNeeded(logger,
                                    baseDir,
                                    "mods/zzz_senpatcher_cs3asset.p3a",
                                    versionStringWithSettings,
                                    [&](SenPatcher::P3APackData& packData) -> bool {
                                        return CollectAssets(logger,
                                                             callback,
                                                             packData.GetMutableFiles(),
                                                             allowSwitchToNightmare);
                                    })
              && success;
    success =
        CreateArchiveIfNeeded(logger,
                              baseDir,
                              "mods/zzz_senpatcher_cs3audio.p3a",
                              versionString,
                              [&](SenPatcher::P3APackData& packData) -> bool {
                                  return CollectAudio(logger, callback, packData.GetMutableFiles());
                              })
        && success;
    success = CreateArchiveIfNeeded(logger,
                                    baseDir,
                                    "mods/zzz_senpatcher_cs3pka.p3a",
                                    versionStringWithPka,
                                    [&](SenPatcher::P3APackData& packData) -> bool {
                                        return CollectPkaAssets(logger,
                                                                callback,
                                                                packData.GetMutableFiles(),
                                                                vanillaPKAs,
                                                                callbackPkaStub);
                                    })
              && success;
    success = CreateVideoIfNeeded(logger,
                                  baseDir,
                                  "data/movie_us/webm/insa_f5.webm",
                                  [&](std::vector<char>& videoData) -> bool {
                                      return SenLib::Sen3::FileFixes::insa05::TryApply(callback,
                                                                                       videoData);
                                  })
              && success;
    success = CreateVideoIfNeeded(logger,
                                  baseDir,
                                  "data/movie_us/webm/insa_f8.webm",
                                  [&](std::vector<char>& videoData) -> bool {
                                      return SenLib::Sen3::FileFixes::insa08::TryApply(callback,
                                                                                       videoData);
                                  })
              && success;
    success = CreateVideoIfNeeded(logger,
                                  baseDir,
                                  "data/movie_us/webm/insa_f9.webm",
                                  [&](std::vector<char>& videoData) -> bool {
                                      return SenLib::Sen3::FileFixes::insa09::TryApply(callback,
                                                                                       videoData);
                                  })
              && success;
    return success;
}
} // namespace SenLib::Sen3
