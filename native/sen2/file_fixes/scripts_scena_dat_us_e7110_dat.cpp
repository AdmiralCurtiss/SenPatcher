#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_e7110[] = "Voice clip fixes in Pantagruel main hall.";
}

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7110_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/e7110.dat",
            166617,
            HyoutaUtils::Hash::SHA1FromHexString("65a894acd9f5ba6f72a5c5f6f336c7cda60a4b9c"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        using namespace HyoutaUtils::MemWrite;

        // okay, so... there's a flashback in here to CS1 file e5010 here. this scene is NOT voiced
        // in CS1, however, Alisa's and Gaius's lines here do have unused voice clips in CS2
        // (pc28v010333, pc28v010334, pc28v090242) which suggests that Crow's line should have one
        // too, but I can't find it. maybe they planned to have these but then Crow had no extra
        // lines to record so they dropped it? anyway, in case this clip exists, here's where to add
        // it.
        //
        // patcher.ExtendPartialCommand(0x53d7, 0xd5, _location_, _data_);
        // patcher.ExtendPartialCommand(0x54d1, 0x52, _location_, _data_);
        // patcher.ExtendPartialCommand(0x5547, 0xbd, _location_, _data_);

        // Rean has 6 different 'excuse me' lines in this file and PC only uses 3 audio clips for
        // them even though there are 6 different ones in the files, so use all of them.
        WriteUInt16(&bin[0xa2ee], 0xff3f);  // pc28v01960 -> pc28v00705
        WriteUInt16(&bin[0x11cf8], 0xff4c); // pc28v00547 -> pc28v01975
        WriteUInt16(&bin[0x18307], 0xff4b); // pc28v01960 -> pc28v01969

        // Same deal with 'I can sense someone inside...', three lines and PC uses the same clip for
        // all three even though there are three takes in the files.
        WriteUInt16(&bin[0x13d10], 0xff4d); // pc28v01957 -> pc28v01981
        WriteUInt16(&bin[0x180f8], 0xff49); // pc28v01957 -> pc28v01966

        // And finally, two instances of 'That sounds like...'
        WriteUInt16(&bin[0x182d1], 0xff4a); // pc28v01959 -> pc28v01968

        // "..." -> "I..." to match voice clip.
        patcher.ExtendPartialCommand(0x1d784, 0x19, 0x1d798, {{0x49}});


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_e7110_dat
