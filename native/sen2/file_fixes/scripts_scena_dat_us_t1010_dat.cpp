#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "sha1.h"
#include "util/memwrite.h"

namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t1010_dat {
std::string_view GetDescription() {
    return "Voice fixes in Class 7 dorm.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t1010.dat",
            491729,
            SenPatcher::SHA1FromHexString("acbda7e71e5622fe0c600792f18d25ad2c4eca0a"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        SenScriptPatcher patcher(bin);

        using namespace HyoutaUtils::MemWrite;

        // remove sigh voice clip; this is weird to have because Rean is unvoiced in this scene
        // otherwise
        patcher.RemovePartialCommand(0x2a14, 0x15, 0x2a17, 5);

        // a bunch of lines where they used duplicate takes across scenes instead of unique ones
        // there's two instances of the final bond scenes and the epilogue evening scenes here:
        // once for the memories menu and once for the actual game; a few have minor inconsistencies
        // that this fixes up too; also note that the epilogue evening scenes have two variants, for
        // if the person is picked as your partner or not picked as your partner, and the memories
        // menu only has duplicates for the former variant

        // laura (final bonding event), pc28v01138 -> pc28v01173
        WriteUInt16(&bin[0x93e6], 0xfefb);
        WriteUInt16(&bin[0x39ace], 0xfefb);

        // fie (final bonding event), one line has two voice clips assigned to it, remove one of
        // them
        patcher.RemovePartialCommand(0xf815, 0x15, 0xf81d, 0x5);
        patcher.RemovePartialCommand(0x42bc9, 0x15, 0x42bd1, 0x5);

        // towa (final bonding event), pc28v01138 -> pc28v01325
        WriteUInt16(&bin[0x15633], 0xff44);
        WriteUInt16(&bin[0x4c6ab], 0xff44);

        // towa (final bonding event), pc28v01332 -> pc28v01341
        // inconsistent between game and memories menu, match memories to game
        WriteUInt16(&bin[0x4dbdb], 0xfef8);

        // towa (final bonding event), pc28v01333 -> pc28v01344
        WriteUInt16(&bin[0x16e36], 0xfef9);
        WriteUInt16(&bin[0x4deae], 0xfef9);

        // machias (final bonding event), single glyph ... -> three dot ...
        WriteArray(&bin[0x1d581], {{0x2e, 0x2e, 0x2e}});
        WriteArray(&bin[0x55585], {{0x2e, 0x2e, 0x2e}});

        // gaius (final bonding event), pc28v01138 -> pc28v01447
        WriteUInt16(&bin[0x20ece], 0xfefa);
        WriteUInt16(&bin[0x5b14a], 0xfefa);

        // sharon (epilogue evening), pc28v01807 -> pc28v01644
        WriteUInt16(&bin[0x2371f], 0xff45);

        // jusis (epilogue evening), pc28v01676 -> pc28v01694
        WriteUInt16(&bin[0x27066], 0xff46);
        WriteUInt16(&bin[0x5a4a4], 0xff46);

        // laura (epilogue evening), pc28v01707 -> pc28v01774
        WriteUInt16(&bin[0x2ca9c], 0xfeff);
        WriteUInt16(&bin[0x3cb18], 0xfeff);

        // laura (epilogue evening), pc28v01648 -> pc28v01779
        WriteUInt16(&bin[0x2cefd], 0xff01);
        WriteUInt16(&bin[0x3cf79], 0xff01);

        // emma (epilogue evening), pc28v01707 -> pc28v01796
        WriteUInt16(&bin[0x2e27e], 0xff47);
        WriteUInt16(&bin[0x4034b], 0xff47);

        // emma (epilogue evening), pc28v01648 -> pc28v01800
        WriteUInt16(&bin[0x2e85e], 0xff48);
        WriteUInt16(&bin[0x4092b], 0xff48);

        // fie (epilogue evening if NOT picked), pc28v01770 -> pc28v01812
        WriteUInt16(&bin[0x2f56e], 0xff05);

        // fie (epilogue evening), pc28v01648 -> pc28v01817
        // inconsistent between game and memories menu, match memories to game
        WriteUInt16(&bin[0x43b64], 0xff06);

        // fie (epilogue evening), pc28v01707 -> pc28v01819
        WriteUInt16(&bin[0x2fcbe], 0xff07);
        WriteUInt16(&bin[0x43ca5], 0xff07);

        // fie (epilogue evening), pc28v01676 -> pc28v01822
        WriteUInt16(&bin[0x2fe07], 0xff09);
        WriteUInt16(&bin[0x43dee], 0xff09);

        // millium (epilogue evening), pc28v01648 -> pc28v01835
        // inconsistent between game and memories menu, match memories to game
        WriteUInt16(&bin[0x47025], 0xff2a);

        // sara (epilogue evening), pc28v01648 -> pc28v01861
        WriteUInt16(&bin[0x325e6], 0xff0a);
        WriteUInt16(&bin[0x4b173], 0xff0a);

        // alfin (final bonding event), 'Oooh...' -> 'Ohhh...'
        // inconsistent between game and memories menu, match memories to game
        WriteArray(&bin[0x4fe0c], {{0x68, 0x68}});

        // gaius (epilogue evening), remove extra space in one line that exists in memories only
        patcher.RemovePartialCommand(0x5ce12, 0x15b, 0x5cebd, 0x1);


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_scena_dat_us_t1010_dat
