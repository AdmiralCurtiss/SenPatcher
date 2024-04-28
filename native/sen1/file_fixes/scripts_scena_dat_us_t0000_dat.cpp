#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/sen_script_patcher.h"
#include "util/hash/sha1.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/stream.h"

namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0000_dat {
std::string_view GetDescription() {
    return "Fix formatting issues on Thors campus grounds.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_us/t0000.dat",
            683998,
            SenPatcher::SHA1FromHexString("83fc174bcce22201fe2053f855e8879b3091e649"));
        if (!file) {
            return false;
        }

        using namespace HyoutaUtils::MemRead;
        using namespace HyoutaUtils::MemWrite;

        auto& bin = file->Data;

        SenScriptPatcher patcher(bin);
        patcher.ReplacePartialCommand(0x1c6e0, 0x8b, 0x1c71f, 3, {{0x27}});
        patcher.ReplacePartialCommand(0x1c6e0, 0x89, 0x1c74f, 3, {{0x27}});

        // formatting issues in Alisa Chapter 1 Evening bonding event
        std::swap(bin[0x79307], bin[0x7930d]);

        // formatting issues in Jusis Chapter 3 Day bonding event
        std::swap(bin[0x7c3a8], bin[0x7c3ab]);
        WriteUInt8(&bin[0x7c3cd], 0x01);
        std::swap(bin[0x7c4b5], bin[0x7c4bb]);
        std::swap(bin[0x7c55a], bin[0x7c55f]);
        std::swap(bin[0x7c585], bin[0x7c58a]);
        std::swap(bin[0x7c651], bin[0x7c659]);
        std::swap(bin[0x7c908], bin[0x7c910]);
        std::swap(bin[0x7c957], bin[0x7c95d]);
        std::swap(bin[0x7c9be], bin[0x7c9c2]);
        std::swap(bin[0x7ce6f], bin[0x7ce74]);

        // formatting issues in Alisa Chapter 5 Day bonding event
        std::swap(bin[0x79ad0], bin[0x79ad6]);
        std::swap(bin[0x7a484], bin[0x7a488]);
        std::swap(bin[0x7a4ab], bin[0x7a4b3]);
        std::swap(bin[0x7a598], bin[0x7a59d]);

        // formatting issues in Fie Chapter 5 Evening bonding event
        std::swap(bin[0x7f942], bin[0x7f947]);
        WriteUInt8(&bin[0x7fcc6], 0x01);

        // formatting issues in Fie Chapter 6 Day bonding event
        std::swap(bin[0x8024c], bin[0x80251]);
        std::swap(bin[0x8080e], bin[0x80814]);

        // formatting issues in Elliot Chapter 6 Day bonding event
        WriteUInt8(&bin[0x7dec6], 0x01);
        std::swap(bin[0x7df0f], bin[0x7df13]);
        std::swap(bin[0x7df36], bin[0x7df3b]);
        std::swap(bin[0x7df7d], bin[0x7df87]);
        std::swap(bin[0x7dff0], bin[0x7dff6]);
        std::swap(bin[0x7e05d], bin[0x7e066]);
        WriteUInt8(&bin[0x7e07f], 0x01);
        std::swap(bin[0x7e14e], bin[0x7e151]);

        // formatting issues in Alisa Chapter 6 Evening bonding event
        WriteUInt8(&bin[0x7adaf], 0x01);
        std::swap(bin[0x7af31], bin[0x7af36]);
        std::swap(bin[0x7b2a5], bin[0x7b2ae]);
        WriteUInt8(&bin[0x7b2ca], 0x01);
        std::swap(bin[0x7b3e2], bin[0x7b3e6]);
        WriteUInt8(&bin[0x7b407], 0x01);
        std::swap(bin[0x7b8f2], bin[0x7b8f5]);
        std::swap(bin[0x7b91b], bin[0x7b91f]);
        std::swap(bin[0x7b98d], bin[0x7b991]);
        std::swap(bin[0x7b9e5], bin[0x7b9ea]);
        std::swap(bin[0x7ba07], bin[0x7ba0f]);

        // fix several lines in the side quests at the festival, this really got messed up somehow
        patcher.ReplacePartialCommand(
            0xa1c1a,
            0x40,
            0xa1c20,
            0x17,
            {{0x4f, 0x68, 0x2c, 0x20, 0x49, 0x20, 0x73, 0x65, 0x65, 0x2e, 0x2e}});
        patcher.ReplacePartialCommand(
            0xa2cf9, 0x1f, 0xa2cff, 0x16, {{0x45, 0x78, 0x63, 0x75, 0x73, 0x65, 0x20, 0x6d,
                                            0x65, 0x2c, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
                                            0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20,
                                            0x73, 0x6f, 0x6e, 0x27, 0x73, 0x01, 0x62, 0x61,
                                            0x6c, 0x6c, 0x6f, 0x6f, 0x6e, 0x20, 0x75, 0x70,
                                            0x20, 0x74, 0x68, 0x65, 0x72, 0x65}});
        patcher.ReplacePartialCommand(0xa353a,
                                      0x41,
                                      0xa3578,
                                      0x1,
                                      {{0x01,
                                        0x61,
                                        0x6e,
                                        0x79,
                                        0x74,
                                        0x68,
                                        0x69,
                                        0x6e,
                                        0x67,
                                        0x20,
                                        0x65,
                                        0x6c,
                                        0x73,
                                        0x65,
                                        0x2e}});

        // fix lowercase 't' at start of sentence
        WriteUInt8(&bin[0xa3ebc], 0x54);

        // course rock salt -> coarse rock salt
        WriteUInt8(&bin[0xa25d2], 0x61);

        // 'or really good guy' -> 'or a really good guy' plus textbox formatting
        {
            const auto data = ReadArray<0x78>(&bin[0x21b9b]);
            std::vector<char> msData;
            MemoryStream ms(msData);
            ms.Write(&data[0], 0x4f);
            ms.Write(&data[0x50], 0x4);
            ms.WriteByte(0x01);
            ms.Write(&data[0x55], 0xd);
            ms.WriteByte(0x61);
            ms.Write(&data[0x61], 0x17);
            patcher.ReplaceCommand(0x21b9b, 0x78, msData);
        }


        result.emplace_back(std::move(bin), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_scena_dat_us_t0000_dat
