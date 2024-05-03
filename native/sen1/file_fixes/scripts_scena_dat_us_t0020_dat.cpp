#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memread.h"
#include "util/memwrite.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0020_dat {
std::string_view GetDescription() {
    return "Fix formatting issues in Thors (top floor).";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0020.dat",
            421144,
            HyoutaUtils::Hash::SHA1FromHexString("9668b944717fe2283a482367f1448ee1fc63e832"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemRead;
        using namespace HyoutaUtils::MemWrite;

        SenScriptPatcher patcher(bin);

        // formatting issues in Gaius Chapter 2 Day bonding event
        std::swap(bin[0x58760], bin[0x58764]);
        std::swap(bin[0x58915], bin[0x58919]);
        std::swap(bin[0x589c3], bin[0x589c8]);
        std::swap(bin[0x589f9], bin[0x589fd]);
        std::swap(bin[0x58a20], bin[0x58a26]);

        // formatting issues in Fie Chapter 2 Evening bonding event
        std::swap(bin[0x577b4], bin[0x577b7]);
        WriteUInt8(&bin[0x57c0c], 0x01);

        // formatting issues in Elliot Chapter 3 Day bonding event
        std::swap(bin[0x55e76], bin[0x55e7d]);
        std::swap(bin[0x56007], bin[0x5600c]);
        WriteUInt8(&bin[0x561fb], 0x01);
        std::swap(bin[0x56238], bin[0x5623d]);
        std::swap(bin[0x5627b], bin[0x56280]);
        std::swap(bin[0x563be], bin[0x563c2]);

        // formatting issues in Alisa Chapter 3 Day bonding event
        std::swap(bin[0x550aa], bin[0x550b1]);
        std::swap(bin[0x550d0], bin[0x550dc]);

        // formatting issues in Elliot Chapter 4 Evening bonding event
        std::swap(bin[0x56913], bin[0x56919]);
        std::swap(bin[0x5693c], bin[0x56940]);
        std::swap(bin[0x56af4], bin[0x56af7]);
        std::swap(bin[0x56b35], bin[0x56b3a]);
        std::swap(bin[0x56d2a], bin[0x56d37]);
        std::swap(bin[0x56d54], bin[0x56d63]);
        std::swap(bin[0x5729d], bin[0x572a1]);

        // formatting issues in Gaius Chapter 6 Day bonding event
        std::swap(bin[0x59357], bin[0x5935c]);
        std::swap(bin[0x59482], bin[0x59486]);
        std::swap(bin[0x5972f], bin[0x59736]);
        std::swap(bin[0x5976f], bin[0x59778]);

        // formatting issues in Crow Chapter 6 Evening bonding event
        std::swap(bin[0x59f0a], bin[0x59f11]);
        std::swap(bin[0x5a022], bin[0x5a027]);
        WriteUInt8(&bin[0x5a048], 0x01);
        std::swap(bin[0x5a1cb], bin[0x5a1ce]);
        std::swap(bin[0x5a1f3], bin[0x5a1fa]);
        std::swap(bin[0x5a240], bin[0x5a245]);
        std::swap(bin[0x5a47b], bin[0x5a480]);
        std::swap(bin[0x5aaa0], bin[0x5aaa6]);
        std::swap(bin[0x5aac6], bin[0x5aad1]);
        std::swap(bin[0x5ac44], bin[0x5ac48]);

        // add comma in Alisa line (voice match) (Final Chapter, scene in classroom after Garellia
        // is destroyed)
        patcher.ExtendPartialCommand(0x49633, 0x19, 0x49642, {{0x2c}});

        // NPC line change from 'sounds good to me' to 'this looks pretty good to me'.
        // this is taken from PS4 and makes more sense, since the guy is looking at the
        // art class exhibits while saying this. (on 10/24, festival after final dungeon)
        auto line = ReadArray<0x11>(&bin[0x58681]);
        line[0] = bin[0x58658];
        patcher.ReplacePartialCommand(0x12d23, 0x17, 0x12d26, 0x6, line);

        // fix typo 'as you made your way around' -> 'as you make your way around'
        WriteUInt8(&bin[0x10b05], 0x6b);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0020_dat
