#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "util/bps.h"
#include "sen/decompress_helper.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen1/tbl.h"
#include "util/hash/sha1.h"

namespace {
static constexpr char PatchData3[] = {
#include "embed_sen1_vctiming_us_3.h"
};
static constexpr size_t PatchLength3 = sizeof(PatchData3);
static constexpr char PatchData4[] = {
#include "embed_sen1_vctiming_us_4.h"
};
static constexpr size_t PatchLength4 = sizeof(PatchData4);
} // namespace

namespace SenLib::Sen1::FileFixes::text_dat_us_t_voice_tbl {
std::string_view GetDescription() {
    return "Add missing lipsync for the extra PC voices.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_us/t_voice.tbl",
            475328,
            HyoutaUtils::Hash::SHA1FromHexString("dc8fa92820abc1b46a646b4d75ba5d239bd22ee9"));
        auto file_timing = getCheckedFile(
            "data/text/dat_us/t_vctiming.tbl",
            122568,
            HyoutaUtils::Hash::SHA1FromHexString("f4b9ff78474452aac44f4b0c07c5a3cc1ce27359"));
        if (!file || !file_timing) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Fix the Sara Finale voice clip
        {
            auto& e = tbl.Entries[13795];
            VoiceData voice(e.Data.data(),
                            e.Data.size(),
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                            HyoutaUtils::TextUtils::GameTextEncoding::UTF8);
            voice.Name = "pc8v10299";
            e.Data = voice.ToBinary(HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                                    HyoutaUtils::TextUtils::GameTextEncoding::UTF8);
        }

        // alternate take of the same line; PS4 uses this one and I agree, it's the better take in
        // that context
        {
            auto& e = tbl.Entries[11247];
            VoiceData voice(e.Data.data(),
                            e.Data.size(),
                            HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                            HyoutaUtils::TextUtils::GameTextEncoding::UTF8);
            voice.Name += "_a";
            e.Data = voice.ToBinary(HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                                    HyoutaUtils::TextUtils::GameTextEncoding::UTF8);
        }

        // We use the PS4 timings for here because several lines (particularly in the Prologue) were
        // re-recorded between PS3 and PC and the PS4 timings should match that better. Note that
        // the PC version by default uses timings that exactly match the PS3 version, so they were
        // not updated for that release. NOTE: I originally thought I needed to copy e8v04585 from 3
        // to 4 since we use the PS3 line there (Emma's "class spirit" line in the final dungeon)
        // but it turns out those have the same timings in both versions, so that's not actually
        // necessary.
        std::vector<char> vctiming4;
        {
            HyoutaUtils::Stream::DuplicatableByteArrayStream vctiming(file_timing->Data.data(),
                                                         file_timing->Data.size());
            auto p3d = DecompressFromBuffer(PatchData3, PatchLength3);
            HyoutaUtils::Stream::DuplicatableByteArrayStream patch3(p3d.data(), p3d.size());
            HyoutaUtils::Stream::DuplicatableByteArrayStream patch4(PatchData4, PatchLength4);
            std::vector<char> vctiming3;
            HyoutaUtils::Bps::ApplyPatchToStream(vctiming, patch3, vctiming3);
            HyoutaUtils::Stream::DuplicatableByteArrayStream tmp(vctiming3.data(), vctiming3.size());
            HyoutaUtils::Bps::ApplyPatchToStream(tmp, patch4, vctiming4);
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tbl.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);
        result.emplace_back(
            std::move(vctiming4), file_timing->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::text_dat_us_t_voice_tbl
