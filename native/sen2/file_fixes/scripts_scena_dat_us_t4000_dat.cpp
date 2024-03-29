#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t4000_dat {
std::string_view GetDescription() {
    return "Fix typo in Ymir.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t4000.dat",
            398073,
            SenPatcher::SHA1FromHexString("8b39f1bd64e65c40194b616b4c43f5d9c4d14c73"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        // extra comma
        // NPC in Act 1 Part 1, before departing for Celdic.
        patcher.RemovePartialCommand(0x1215f, 0x99, 0x121a1, 0x1);

        // PS4 changed two 'someday' to 'some day' but as far as I can tell the former is fine for
        // an unspecified time point?
        // using (MemoryStream ms = new MemoryStream()) {
        //    bin.Position = 0x15989;
        //    ms.Write(bin.ReadBytes(0xe0));
        //    bin.DiscardBytes(1);
        //    ms.Write(bin.ReadBytes(0x77));
        //    ms.WriteByte(0x20);
        //    ms.Write(bin.ReadBytes(0x10));
        //    patcher.ReplaceCommand(0x15989, 0x168, ms.CopyToByteArrayAndDispose());
        // }

        // I know this is a lot of ask -> I know this is a lot to ask
        // Patiry/Kargo sidequest in Act 2 Part 1
        HyoutaUtils::MemWrite::WriteArray(&bin[0x43206], {{0x74, 0x6f}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t4000_dat
