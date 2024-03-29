#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "bps.h"
#include "decompress_helper.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen1/tbl.h"
#include "sha1.h"

namespace {
static constexpr char PatchData3[] = {
#include "embed_sen1_vctiming_jp_3.h"
};
static constexpr size_t PatchLength3 = sizeof(PatchData3);
static constexpr char PatchData4[] = {
#include "embed_sen1_vctiming_jp_4.h"
};
static constexpr size_t PatchLength4 = sizeof(PatchData4);
} // namespace

namespace SenLib::Sen1::FileFixes::text_dat_t_voice_tbl {
std::string_view GetDescription() {
    return "Fix Japanese voice tables. This is half of a fix for JP voice lipsync.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat/t_voice.tbl",
            470388,
            SenPatcher::SHA1FromHexString("90a5c55ed954d77111563b9f3bb62ce7c534135a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // remove the new-for-PC english voice clips from the JP voice table
        tbl.Entries.erase(tbl.Entries.begin() + 9495, tbl.Entries.end());

        // the PS3 and PS4 versions disagree slightly on some lines, I took the PS3 timings here but
        // maybe the PS4 ones are better?
        std::vector<char> vctiming3;
        {
            std::vector<char> vctiming4 = DecompressFromBuffer(PatchData4, PatchLength4);
            auto p3d = DecompressFromBuffer(PatchData3, PatchLength3);
            SenLib::DuplicatableByteArrayStream tmp(vctiming4.data(), vctiming4.size());
            SenLib::DuplicatableByteArrayStream patch3(p3d.data(), p3d.size());
            HyoutaUtils::Bps::ApplyPatchToStream(tmp, patch3, vctiming3);
        }

        std::vector<char> bin2;
        MemoryStream ms(bin2);
        tbl.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(std::move(vctiming3),
                            SenPatcher::InitializeP3AFilename("data/text/dat/t_vctiming.tbl"),
                            SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::text_dat_t_voice_tbl
