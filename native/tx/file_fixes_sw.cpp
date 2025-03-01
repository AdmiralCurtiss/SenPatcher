#include "file_fixes_sw.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "modload/loaded_mods.h"
#include "p3a/pack.h"
#include "p3a/util.h"
#include "sen/asset_patch.h"
#include "sen/file_getter.h"
#include "tx/bra.h"
#include "tx/bra_extract.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/logger.h"
#include "util/text.h"

#define DECLARE_STANDARD_FIXSW(name)                                            \
    namespace SenLib::TX::FileFixesSw::##name {                                 \
        bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile, \
                      std::vector<SenPatcher::P3APackFile>& result);            \
    }
DECLARE_STANDARD_FIXSW(e0000)
DECLARE_STANDARD_FIXSW(e0100)
DECLARE_STANDARD_FIXSW(e3600)
DECLARE_STANDARD_FIXSW(e6000)
DECLARE_STANDARD_FIXSW(e9100)
DECLARE_STANDARD_FIXSW(m1100)
DECLARE_STANDARD_FIXSW(m2190)
DECLARE_STANDARD_FIXSW(m3190)
DECLARE_STANDARD_FIXSW(m3800)
DECLARE_STANDARD_FIXSW(m3801)
DECLARE_STANDARD_FIXSW(m5290)
DECLARE_STANDARD_FIXSW(m5319)
DECLARE_STANDARD_FIXSW(m5800)
DECLARE_STANDARD_FIXSW(m5801)
DECLARE_STANDARD_FIXSW(m6309)
DECLARE_STANDARD_FIXSW(m8209)
DECLARE_STANDARD_FIXSW(m8300)
DECLARE_STANDARD_FIXSW(m8331)
DECLARE_STANDARD_FIXSW(m8390)
DECLARE_STANDARD_FIXSW(m9090)
DECLARE_STANDARD_FIXSW(m9800)
DECLARE_STANDARD_FIXSW(m9801)
DECLARE_STANDARD_FIXSW(m9810)
DECLARE_STANDARD_FIXSW(m9820)
DECLARE_STANDARD_FIXSW(m9830)
DECLARE_STANDARD_FIXSW(m9860)
DECLARE_STANDARD_FIXSW(m9870)
DECLARE_STANDARD_FIXSW(s1000)
DECLARE_STANDARD_FIXSW(s1100)
DECLARE_STANDARD_FIXSW(s2000)
DECLARE_STANDARD_FIXSW(s2010)
DECLARE_STANDARD_FIXSW(s2100)
DECLARE_STANDARD_FIXSW(s2910)
DECLARE_STANDARD_FIXSW(s3000)
DECLARE_STANDARD_FIXSW(s3001)
DECLARE_STANDARD_FIXSW(s3002)
DECLARE_STANDARD_FIXSW(s3100)
DECLARE_STANDARD_FIXSW(s3110)
DECLARE_STANDARD_FIXSW(s3120)
DECLARE_STANDARD_FIXSW(s4000)
DECLARE_STANDARD_FIXSW(s4100)
DECLARE_STANDARD_FIXSW(s5000)
DECLARE_STANDARD_FIXSW(s5100)
DECLARE_STANDARD_FIXSW(s6000)
DECLARE_STANDARD_FIXSW(s6100)
DECLARE_STANDARD_FIXSW(s6110)
DECLARE_STANDARD_FIXSW(s7000)
DECLARE_STANDARD_FIXSW(s7100)
DECLARE_STANDARD_FIXSW(s7110)
DECLARE_STANDARD_FIXSW(s7120)
DECLARE_STANDARD_FIXSW(s7130)
DECLARE_STANDARD_FIXSW(s7200)
DECLARE_STANDARD_FIXSW(s8000)
DECLARE_STANDARD_FIXSW(system_dat)
DECLARE_STANDARD_FIXSW(t_active)
DECLARE_STANDARD_FIXSW(t_dlc)
DECLARE_STANDARD_FIXSW(t_dungeon)
DECLARE_STANDARD_FIXSW(t_hikitugi)
DECLARE_STANDARD_FIXSW(t_item)
DECLARE_STANDARD_FIXSW(t_itemhelp)
DECLARE_STANDARD_FIXSW(t_jump)
DECLARE_STANDARD_FIXSW(t_magic)
DECLARE_STANDARD_FIXSW(t_main)
DECLARE_STANDARD_FIXSW(t_notechar)
DECLARE_STANDARD_FIXSW(t_notehelp)
DECLARE_STANDARD_FIXSW(t_notemons)
DECLARE_STANDARD_FIXSW(t_orblv)
DECLARE_STANDARD_FIXSW(t_place)
DECLARE_STANDARD_FIXSW(t_quest)
DECLARE_STANDARD_FIXSW(t_text)
DECLARE_STANDARD_FIXSW(t1000)
DECLARE_STANDARD_FIXSW(t1001)
DECLARE_STANDARD_FIXSW(t1110)
DECLARE_STANDARD_FIXSW(t1120)
DECLARE_STANDARD_FIXSW(t1130)
DECLARE_STANDARD_FIXSW(t1210)
DECLARE_STANDARD_FIXSW(t1310)
DECLARE_STANDARD_FIXSW(t1410)
DECLARE_STANDARD_FIXSW(t2000)
DECLARE_STANDARD_FIXSW(t2010)
DECLARE_STANDARD_FIXSW(t2100)
DECLARE_STANDARD_FIXSW(t2200)
DECLARE_STANDARD_FIXSW(t2300)
DECLARE_STANDARD_FIXSW(t2400)
DECLARE_STANDARD_FIXSW(t3000)
DECLARE_STANDARD_FIXSW(t3100)
DECLARE_STANDARD_FIXSW(t3200)
DECLARE_STANDARD_FIXSW(t3300)
DECLARE_STANDARD_FIXSW(t4000)
DECLARE_STANDARD_FIXSW(t4100)
DECLARE_STANDARD_FIXSW(t4200)
DECLARE_STANDARD_FIXSW(t4300)
DECLARE_STANDARD_FIXSW(t4400)
DECLARE_STANDARD_FIXSW(t5000)
DECLARE_STANDARD_FIXSW(t5110)
DECLARE_STANDARD_FIXSW(t5120)
DECLARE_STANDARD_FIXSW(t5130)
DECLARE_STANDARD_FIXSW(t6000)
DECLARE_STANDARD_FIXSW(t6010)
DECLARE_STANDARD_FIXSW(t6100)
DECLARE_STANDARD_FIXSW(t6200)
DECLARE_STANDARD_FIXSW(t6300)
DECLARE_STANDARD_FIXSW(t6400)
DECLARE_STANDARD_FIXSW(t6500)
DECLARE_STANDARD_FIXSW(t6600)
DECLARE_STANDARD_FIXSW(t6700)
DECLARE_STANDARD_FIXSW(tk_akie)
DECLARE_STANDARD_FIXSW(tk_gorou)
DECLARE_STANDARD_FIXSW(tk_haruhiko)
DECLARE_STANDARD_FIXSW(tk_hekiru)
DECLARE_STANDARD_FIXSW(tk_nodoka)
DECLARE_STANDARD_FIXSW(tk_rion)

#define TRY_APPLY_SW(asset, apply)                      \
    do {                                                \
        logger.Log("Applying: ");                       \
        logger.Log(#asset);                             \
        logger.Log("\n");                               \
        if (!(SenLib::TX::FileFixesSw::asset::apply)) { \
            logger.Log(" -- Failed!\n");                \
            return false;                               \
        }                                               \
    } while (false)

namespace {
static constexpr char Data_SenpatcherModIniEnglish[] = {
    (char)0x5B, (char)0x54, (char)0x58, (char)0x4D, (char)0x6F, (char)0x64, (char)0x5D,
    (char)0x0D, (char)0x0A, (char)0x4C, (char)0x61, (char)0x6E, (char)0x67, (char)0x75,
    (char)0x61, (char)0x67, (char)0x65, (char)0x3D, (char)0x45, (char)0x6E, (char)0x67,
    (char)0x6C, (char)0x69, (char)0x73, (char)0x68, (char)0x0D, (char)0x0A};
static constexpr size_t Length_SenpatcherModIniEnglish = sizeof(Data_SenpatcherModIniEnglish);

static std::string_view StripTrailingNull(std::string_view sv) {
    std::string_view s = sv;
    while (!s.empty() && s.back() == '\0') {
        s = s.substr(0, s.size() - 1);
    }
    return s;
}
} // namespace

namespace SenLib::TX {
static bool AddDifferentSwitchFilesRecursive(HyoutaUtils::Logger& logger,
                                             std::string_view dataDirSwitch,
                                             const SenPatcher::GetCheckedFileCallback& callback,
                                             std::vector<SenPatcher::P3APackFile>& packFiles,
                                             std::string_view relativePath) {
    std::error_code ec;
    std::string currentDir;
    currentDir.append(dataDirSwitch);
    currentDir.push_back('/');
    currentDir.append(relativePath);
    std::filesystem::directory_iterator iterator(
        std::u8string_view((const char8_t*)currentDir.data(),
                           ((const char8_t*)currentDir.data()) + currentDir.size()),
        ec);
    if (ec) {
        return false;
    }
    for (auto const& entry : iterator) {
        std::string relative;
        relative.append(relativePath);
        relative.push_back('/');
        const auto filenameAsPath = entry.path().filename().u8string();
        relative.append(
            std::string_view((const char*)filenameAsPath.data(),
                             ((const char*)filenameAsPath.data()) + filenameAsPath.size()));

        if (entry.is_directory()) {
            if (!AddDifferentSwitchFilesRecursive(
                    logger, dataDirSwitch, callback, packFiles, relative)) {
                return false;
            }
            continue;
        }

        std::array<char, 0x100> fn{};
        if (!SenPatcher::CopyToP3AFilename(fn, relative)) {
            return false;
        }
        fn = SenPatcher::NormalizeP3AFilename(fn);

        logger.Log("Checking Switch file: ").Log(relative).Log("\n");

        HyoutaUtils::IO::File file(entry.path(), HyoutaUtils::IO::OpenMode::Read);
        if (!file.IsOpen()) {
            return false;
        }
        auto filesize = file.GetLength();
        if (!filesize) {
            return false;
        }
        if (*filesize > std::numeric_limits<size_t>::max()) {
            return false;
        }
        std::vector<char> filedata;
        filedata.resize(static_cast<size_t>(*filesize));
        if (file.Read(filedata.data(), filedata.size()) != filedata.size()) {
            return false;
        }

        // check if this file exists in the PC files
        auto hash = HyoutaUtils::Hash::CalculateSHA1(filedata.data(), filedata.size());
        auto pcFiledata = callback(
            StripTrailingNull(std::string_view(fn.data(), fn.size())), filedata.size(), hash);
        if (!pcFiledata || pcFiledata->Data != filedata) {
            logger.Log("Not in PC files\n");
            packFiles.emplace_back(std::move(filedata), fn, SenPatcher::P3ACompressionType::LZ4);
            continue;
        }

        // is in PC files, don't need to pack it
    }
    return true;
}

static bool CollectAssetsSwitch(HyoutaUtils::Logger& logger,
                                std::string_view dataDirSwitch,
                                const SenPatcher::GetCheckedFileCallback& callback,
                                std::vector<SenPatcher::P3APackFile>& packFiles) {
    packFiles.emplace_back(
        std::vector<char>(Data_SenpatcherModIniEnglish,
                          Data_SenpatcherModIniEnglish + Length_SenpatcherModIniEnglish),
        SenPatcher::InitializeP3AFilename("senpatcher_mod.ini"),
        SenPatcher::P3ACompressionType::None);

    if (!AddDifferentSwitchFilesRecursive(logger, dataDirSwitch, callback, packFiles, "scripts")) {
        return false;
    }
    if (!AddDifferentSwitchFilesRecursive(logger, dataDirSwitch, callback, packFiles, "text")) {
        return false;
    }
    return true;
}

static bool ApplyFixesSwitch(HyoutaUtils::Logger& logger,
                             const SenPatcher::GetCheckedFileCallback& callback,
                             std::vector<SenPatcher::P3APackFile>& packFiles) {
    TRY_APPLY_SW(e0000, TryApply(callback, packFiles));
    TRY_APPLY_SW(e0100, TryApply(callback, packFiles));
    TRY_APPLY_SW(e3600, TryApply(callback, packFiles));
    TRY_APPLY_SW(e6000, TryApply(callback, packFiles));
    TRY_APPLY_SW(e9100, TryApply(callback, packFiles));
    TRY_APPLY_SW(m1100, TryApply(callback, packFiles));
    TRY_APPLY_SW(m2190, TryApply(callback, packFiles));
    TRY_APPLY_SW(m3190, TryApply(callback, packFiles));
    TRY_APPLY_SW(m3800, TryApply(callback, packFiles));
    TRY_APPLY_SW(m3801, TryApply(callback, packFiles));
    TRY_APPLY_SW(m5290, TryApply(callback, packFiles));
    TRY_APPLY_SW(m5319, TryApply(callback, packFiles));
    TRY_APPLY_SW(m5800, TryApply(callback, packFiles));
    TRY_APPLY_SW(m5801, TryApply(callback, packFiles));
    TRY_APPLY_SW(m6309, TryApply(callback, packFiles));
    TRY_APPLY_SW(m8209, TryApply(callback, packFiles));
    TRY_APPLY_SW(m8300, TryApply(callback, packFiles));
    TRY_APPLY_SW(m8331, TryApply(callback, packFiles));
    TRY_APPLY_SW(m8390, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9090, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9800, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9801, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9810, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9820, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9830, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9860, TryApply(callback, packFiles));
    TRY_APPLY_SW(m9870, TryApply(callback, packFiles));
    TRY_APPLY_SW(s1000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s1100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s2000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s2010, TryApply(callback, packFiles));
    TRY_APPLY_SW(s2100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s2910, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3001, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3002, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3110, TryApply(callback, packFiles));
    TRY_APPLY_SW(s3120, TryApply(callback, packFiles));
    TRY_APPLY_SW(s4000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s4100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s5000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s5100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s6000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s6100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s6110, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7000, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7100, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7110, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7120, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7130, TryApply(callback, packFiles));
    TRY_APPLY_SW(s7200, TryApply(callback, packFiles));
    TRY_APPLY_SW(s8000, TryApply(callback, packFiles));
    TRY_APPLY_SW(system_dat, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_active, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_dlc, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_dungeon, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_hikitugi, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_item, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_itemhelp, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_jump, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_magic, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_main, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_notechar, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_notehelp, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_notemons, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_orblv, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_place, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_quest, TryApply(callback, packFiles));
    TRY_APPLY_SW(t_text, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1001, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1110, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1120, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1130, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1210, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1310, TryApply(callback, packFiles));
    TRY_APPLY_SW(t1410, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2010, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2100, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2200, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2300, TryApply(callback, packFiles));
    TRY_APPLY_SW(t2400, TryApply(callback, packFiles));
    TRY_APPLY_SW(t3000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t3100, TryApply(callback, packFiles));
    TRY_APPLY_SW(t3200, TryApply(callback, packFiles));
    TRY_APPLY_SW(t3300, TryApply(callback, packFiles));
    TRY_APPLY_SW(t4000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t4100, TryApply(callback, packFiles));
    TRY_APPLY_SW(t4200, TryApply(callback, packFiles));
    TRY_APPLY_SW(t4300, TryApply(callback, packFiles));
    TRY_APPLY_SW(t4400, TryApply(callback, packFiles));
    TRY_APPLY_SW(t5000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t5110, TryApply(callback, packFiles));
    TRY_APPLY_SW(t5120, TryApply(callback, packFiles));
    TRY_APPLY_SW(t5130, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6000, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6010, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6100, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6200, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6300, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6400, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6500, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6600, TryApply(callback, packFiles));
    TRY_APPLY_SW(t6700, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_akie, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_gorou, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_haruhiko, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_hekiru, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_nodoka, TryApply(callback, packFiles));
    TRY_APPLY_SW(tk_rion, TryApply(callback, packFiles));
    return true;
}

SenPatcher::P3APackFile* FindAlreadyPackedFile(std::vector<SenPatcher::P3APackFile>& packFiles,
                                               std::string_view path,
                                               size_t size,
                                               const HyoutaUtils::Hash::SHA1& hash) {
    for (auto& pf : packFiles) {
        if (!pf.HasVectorData()) {
            continue;
        }
        auto& data = pf.GetVectorData();
        if (data.size() != size) {
            continue;
        }
        auto& pfname = pf.GetFilename();
        if (StripTrailingNull(std::string_view(pfname.data(), pfname.size())) != path) {
            continue;
        }
        if (HyoutaUtils::Hash::CalculateSHA1(data.data(), data.size()) != hash) {
            continue;
        }
        return &pf;
    }
    return nullptr;
}

bool CreateSwitchScriptArchive(HyoutaUtils::Logger& logger,
                               std::string_view dataDirSwitch,
                               std::string_view baseDirPc,
                               SenLib::ModLoad::LoadedP3AData& vanillaP3As,
                               SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                               std::span<const std::string_view> pkaPrefixes) {
    // lazy init these so that they won't get inited if no one asks for a file
    bool allowJapaneseBra = false;
    std::optional<SenPatcher::BRA> asset1_bra;
    std::optional<SenPatcher::BRA> asset2_bra;
    std::optional<SenPatcher::BRA> asset3_bra;
    std::optional<SenPatcher::BRA> asset4_bra;
    std::optional<SenPatcher::BRA> audio_bra;
    std::optional<SenPatcher::BRA> japanese_bra;
    std::optional<SenPatcher::BRA> system_bra;
    const auto get_bra = [&](std::optional<SenPatcher::BRA>& bra,
                             std::string_view bra_filename) -> SenPatcher::BRA& {
        if (!bra) {
            std::string fullPath;
            fullPath.reserve(baseDirPc.size() + 1 + bra_filename.size());
            fullPath.append(baseDirPc);
            fullPath.push_back('/');
            fullPath.append(bra_filename);

            bra.emplace();
            bra->Load(fullPath);
            for (size_t i = 0; i < bra->FileCount; ++i) {
                SenLib::ModLoad::FilterP3APath(bra->FileInfo[i].Path.data(),
                                               bra->FileInfo[i].Path.size());
            }
        }
        return *bra;
    };
    const auto get_file_from_bra =
        [](SenPatcher::BRA& bra,
           std::string_view path,
           size_t size,
           const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        if (path.size() > 0x60) {
            return std::nullopt;
        }

        std::array<char, 0x60> tmp{};
        static_assert(sizeof(tmp) == sizeof(SenPatcher::BRAFileInfo::Path));
        std::memcpy(tmp.data(), path.data(), path.size());
        for (size_t i = 0; i < bra.FileCount; ++i) {
            const auto& fi = bra.FileInfo[i];
            const uint32_t compressedSize = fi.CompressedSize;
            const uint32_t uncompressedSize = fi.UncompressedSize;
            if (uncompressedSize == size
                && std::memcmp(tmp.data(), fi.Path.data(), tmp.size()) == 0) {
                auto r = [&]() -> std::optional<SenPatcher::CheckedFileResult> {
                    SenPatcher::CheckedFileResult result;
                    if (!SenPatcher::CopyToP3AFilename(result.Filename, path)) {
                        return std::nullopt;
                    }
                    result.Data.resize(uncompressedSize);
                    auto compressedBuffer = std::make_unique_for_overwrite<char[]>(compressedSize);
                    if (!bra.FileHandle.SetPosition(fi.DataPosition)) {
                        return std::nullopt;
                    }
                    if (bra.FileHandle.Read(compressedBuffer.get(), compressedSize)
                        != compressedSize) {
                        return std::nullopt;
                    }
                    if (!SenPatcher::ExtractAndDecompressBraFile(result.Data.data(),
                                                                 uncompressedSize,
                                                                 compressedBuffer.get(),
                                                                 compressedSize)) {
                        return std::nullopt;
                    }
                    if (HyoutaUtils::Hash::CalculateSHA1(result.Data.data(), uncompressedSize)
                        != hash) {
                        return std::nullopt;
                    }
                    return result;
                }();
                if (r) {
                    return r;
                }
            }
        }
        return std::nullopt;
    };

    const SenPatcher::GetCheckedFileCallback callback =
        [&](std::string_view path,
            size_t size,
            const HyoutaUtils::Hash::SHA1& hash) -> std::optional<SenPatcher::CheckedFileResult> {
        if (allowJapaneseBra) {
            auto fJ = get_file_from_bra(get_bra(japanese_bra, "Japanese.bra"), path, size, hash);
            if (fJ) {
                logger.Log("Found ").Log(path).Log(" in Japanese.bra\n");
                return fJ;
            }
        }
        auto fA = get_file_from_bra(get_bra(audio_bra, "Audio.bra"), path, size, hash);
        if (fA) {
            logger.Log("Found ").Log(path).Log(" in Audio.bra\n");
            return fA;
        }
        auto fS = get_file_from_bra(get_bra(system_bra, "System.bra"), path, size, hash);
        if (fS) {
            logger.Log("Found ").Log(path).Log(" in System.bra\n");
            return fS;
        }
        auto f1 = get_file_from_bra(get_bra(asset1_bra, "Asset1.bra"), path, size, hash);
        if (f1) {
            logger.Log("Found ").Log(path).Log(" in Asset1.bra\n");
            return f1;
        }
        auto f2 = get_file_from_bra(get_bra(asset2_bra, "Asset2.bra"), path, size, hash);
        if (f2) {
            logger.Log("Found ").Log(path).Log(" in Asset2.bra\n");
            return f2;
        }
        auto f3 = get_file_from_bra(get_bra(asset3_bra, "Asset3.bra"), path, size, hash);
        if (f3) {
            logger.Log("Found ").Log(path).Log(" in Asset3.bra\n");
            return f3;
        }
        auto f4 = get_file_from_bra(get_bra(asset4_bra, "Asset4.bra"), path, size, hash);
        if (f4) {
            logger.Log("Found ").Log(path).Log(" in Asset4.bra\n");
            return f4;
        }
        return GetCheckedFile(
            baseDirPc, vanillaP3As, vanillaPKAs, pkaPrefixes, nullptr, path, size, hash);
    };

    std::string_view archivePath = "mods/tx_switch_script.p3a";

    std::string fullArchivePath;
    fullArchivePath.reserve(baseDirPc.size() + 1 + archivePath.size());
    fullArchivePath.append(baseDirPc);
    fullArchivePath.push_back('/');
    fullArchivePath.append(archivePath);

    logger.Log("Creating Switch script archive.\n");
    std::string tmpPath;
    tmpPath.reserve(fullArchivePath.size() + 4);
    tmpPath.append(fullArchivePath);
    tmpPath.append(".tmp");
    HyoutaUtils::IO::File newArchive(std::string_view(tmpPath), HyoutaUtils::IO::OpenMode::Write);
    if (!newArchive.IsOpen()) {
        logger.Log("Opening new archive failed.\n");
        return false;
    }

    SenPatcher::P3APackData packData;
    packData.SetVersion(SenPatcher::P3AHighestSupportedVersion);
    packData.SetAlignment(0x10);

    // files must be in the english-loaded archives to count as existing
    allowJapaneseBra = false;
    if (!CollectAssetsSwitch(logger, dataDirSwitch, callback, packData.GetMutableFiles())) {
        logger.Log("Collecting failed.\n");
        newArchive.Delete();
        return false;
    }

    // once we're applying fixes it's okay to load JP files
    allowJapaneseBra = true;
    if (!ApplyFixesSwitch(logger, callback, packData.GetMutableFiles())) {
        logger.Log("Applying fixes failed.\n");
        newArchive.Delete();
        return false;
    }

    auto& packFiles = packData.GetMutableFiles();
    for (auto& pf : packFiles) {
        auto normalized = SenPatcher::NormalizeP3AFilename(pf.GetFilename());
        pf.SetFilename(normalized);
    }
    std::stable_sort(packFiles.begin(),
                     packFiles.end(),
                     [](const SenPatcher::P3APackFile& lhs, const SenPatcher::P3APackFile& rhs) {
                         const auto& l = lhs.GetFilename();
                         const auto& r = rhs.GetFilename();
                         return memcmp(l.data(), r.data(), l.size()) < 0;
                     });
    if (!SenPatcher::PackP3A(newArchive, packData, 1)) {
        logger.Log("Packing failed.\n");
        newArchive.Delete();
        return false;
    }

    if (!newArchive.Rename(std::string_view(fullArchivePath))) {
        logger.Log("Renaming failed.\n");
        newArchive.Delete();
        return false;
    }

    logger.Log("Created archive!\n");
    return true;
}
} // namespace SenLib::TX
