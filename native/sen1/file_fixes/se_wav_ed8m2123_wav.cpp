#include <string_view>
#include <vector>

#include "sen/file_getter.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "util/hash/sha1.h"

namespace SenLib::Sen1::FileFixes::se_wav_ed8m2123_wav {
std::string_view GetDescription() {
    return "Fix incorrect filename of ed8m2123.wav";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        // just copy the file to a different location
        // we do this because the existing file in the game assets uses a fullwidth lowercase 'm'
        // but the se table asks for a regular ASCII 'm'.
        // note that I'm unsure if this is actually required, as I couldn't locate the usage of this
        // sound effect, but might as well...


        // the Steam/GoG/Humble releases don't agree on the filename encoding, hence the multiple
        // checks here...
        static constexpr auto sha =
            HyoutaUtils::Hash::SHA1FromHexString("684cc74b0837ff1408124f8b8a05cfd9c9a09195");
        auto file = getCheckedFile(
            "data/se/wav/ed8"
            "\xef\xbd\x8d"
            "2123.wav",
            735228,
            sha);
        if (!file) {
            file = getCheckedFile(
                "data/se/wav/ed8"
                "\xc3\xa9\xc5\xb9"
                "2123.wav",
                735228,
                sha);
            if (!file) {
                file = getCheckedFile(
                    "data/se/wav/ed8"
                    "\xc3\xa9\xc3\xac"
                    "2123.wav",
                    735228,
                    sha);
                if (!file) {
                    // tbqh this isn't super important, if we can't find it oh well
                    return true;
                }
            }
        }

        auto& bin = file->Data;
        result.emplace_back(std::move(bin),
                            SenPatcher::InitializeP3AFilename("data/se/wav/ed8m2123.wav"),
                            SenPatcher::P3ACompressionType::None);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::se_wav_ed8m2123_wav
