#include <optional>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen2/tbl.h"
#include "util/hash/sha1.h"

extern "C" {
// This is implied by references to fixed voice clips.
// __declspec(dllexport) char SenPatcherFix_0_voice[] =
//     "Adjust voice file to ID mappings (required for some voice fixes).";
}

namespace {
struct VoiceReplaceInfo {
    std::string_view Name;
    uint16_t VoiceId;
    std::optional<int> ReplaceIndex;
};
} // namespace

namespace SenLib::Sen2::FileFixes::text_dat_us_t_voice_tbl {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/text/dat_us/t_voice.tbl",
            543081,
            HyoutaUtils::Hash::SHA1FromHexString("9019e12bf1d93039254892c87f70a572d2ffc248"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;
        Tbl tbl(bin.data(), bin.size(), HyoutaUtils::EndianUtils::Endianness::LittleEndian);


        // add unmapped entries into the voice table; replace a few that are defined twice while
        // we're here too
        static constexpr auto list = {
            VoiceReplaceInfo{.Name = "pc28v00110", .VoiceId = 65323, .ReplaceIndex = 15736},
            VoiceReplaceInfo{.Name = "pc28v00113", .VoiceId = 65324, .ReplaceIndex = 15737},
            VoiceReplaceInfo{.Name = "pc28v00232", .VoiceId = 65325, .ReplaceIndex = 15738},
            VoiceReplaceInfo{.Name = "pc28v00374", .VoiceId = 65326, .ReplaceIndex = 15740},
            VoiceReplaceInfo{.Name = "pc28v00376", .VoiceId = 65327, .ReplaceIndex = 15741},
            VoiceReplaceInfo{.Name = "pc28v00378", .VoiceId = 65328, .ReplaceIndex = 15742},
            VoiceReplaceInfo{.Name = "pc28v00379", .VoiceId = 65329},
            VoiceReplaceInfo{.Name = "pc28v00380", .VoiceId = 65330},
            VoiceReplaceInfo{.Name = "pc28v00384", .VoiceId = 65331},
            VoiceReplaceInfo{.Name = "pc28v00394", .VoiceId = 65332},
            VoiceReplaceInfo{.Name = "pc28v00396", .VoiceId = 65333},
            VoiceReplaceInfo{.Name = "pc28v00424", .VoiceId = 65334},
            VoiceReplaceInfo{.Name = "pc28v00430", .VoiceId = 65335},
            VoiceReplaceInfo{.Name = "pc28v00431", .VoiceId = 65336},
            VoiceReplaceInfo{.Name = "pc28v00434", .VoiceId = 65337},
            VoiceReplaceInfo{.Name = "pc28v00441", .VoiceId = 65338},
            VoiceReplaceInfo{.Name = "pc28v00444", .VoiceId = 65339},
            VoiceReplaceInfo{.Name = "pc28v00445", .VoiceId = 65340},
            VoiceReplaceInfo{.Name = "pc28v00449", .VoiceId = 65341},
            VoiceReplaceInfo{.Name = "pc28v00450", .VoiceId = 65342},
            VoiceReplaceInfo{.Name = "pc28v00705", .VoiceId = 65343},
            VoiceReplaceInfo{.Name = "pc28v010333", .VoiceId = 65344},
            VoiceReplaceInfo{.Name = "pc28v010334", .VoiceId = 65345},
            VoiceReplaceInfo{.Name = "pc28v010448", .VoiceId = 65346},
            VoiceReplaceInfo{.Name = "pc28v01055", .VoiceId = 65347},
            VoiceReplaceInfo{.Name = "pc28v01325", .VoiceId = 65348},
            VoiceReplaceInfo{.Name = "pc28v01644", .VoiceId = 65349},
            VoiceReplaceInfo{.Name = "pc28v01694", .VoiceId = 65350},
            VoiceReplaceInfo{.Name = "pc28v01796", .VoiceId = 65351},
            VoiceReplaceInfo{.Name = "pc28v01800", .VoiceId = 65352},
            VoiceReplaceInfo{.Name = "pc28v01966", .VoiceId = 65353},
            VoiceReplaceInfo{.Name = "pc28v01968", .VoiceId = 65354},
            VoiceReplaceInfo{.Name = "pc28v01969", .VoiceId = 65355},
            VoiceReplaceInfo{.Name = "pc28v01975", .VoiceId = 65356},
            VoiceReplaceInfo{.Name = "pc28v01981", .VoiceId = 65357},
            VoiceReplaceInfo{.Name = "pc28v020309", .VoiceId = 65358},
            VoiceReplaceInfo{.Name = "pc28v060407", .VoiceId = 65359},
            VoiceReplaceInfo{.Name = "pc28v080364", .VoiceId = 65360},
            VoiceReplaceInfo{.Name = "pc28v080401", .VoiceId = 65361},
            VoiceReplaceInfo{.Name = "pc28v090242", .VoiceId = 65362},
            VoiceReplaceInfo{.Name = "pc28v090265", .VoiceId = 65363},
            VoiceReplaceInfo{.Name = "pc28v100377", .VoiceId = 65364},
            VoiceReplaceInfo{.Name = "pc28v100378", .VoiceId = 65365},
            VoiceReplaceInfo{.Name = "pc28v110354", .VoiceId = 65366},
            VoiceReplaceInfo{.Name = "pc28v110357", .VoiceId = 65367},
            VoiceReplaceInfo{.Name = "pc28v250333", .VoiceId = 65368},
            VoiceReplaceInfo{.Name = "pc28v260310", .VoiceId = 65369},
            VoiceReplaceInfo{.Name = "pc28v90385", .VoiceId = 65370},
            VoiceReplaceInfo{.Name = "pc28v90403", .VoiceId = 65371},
            VoiceReplaceInfo{.Name = "pc28v950034", .VoiceId = 65372},
            VoiceReplaceInfo{.Name = "pc28v950114", .VoiceId = 65373},
            VoiceReplaceInfo{.Name = "pc28v950134", .VoiceId = 65374},
        };
        for (const auto& vri : list) {
            auto entry = tbl.Entries[vri.ReplaceIndex.value_or(0)];
            VoiceData item(entry.Data.data(), entry.Data.size());
            item.name = vri.Name;
            item.idx = vri.VoiceId;
            entry.Data = item.ToBinary();
            if (vri.ReplaceIndex.has_value()) {
                tbl.Entries[*vri.ReplaceIndex] = entry;
            } else {
                tbl.Entries.emplace_back(entry);
            }
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        tbl.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::text_dat_us_t_voice_tbl
