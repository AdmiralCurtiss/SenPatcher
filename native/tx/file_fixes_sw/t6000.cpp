#include <span>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"

#define STR_SPAN(text) std::span<const char>(text, sizeof(text) - 1)

namespace SenLib::TX::FileFixesSw::t6000 {
std::string_view GetDescription() {
    return "Text fixes in Houraichou.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t6000.dat",
            258873,
            HyoutaUtils::Hash::SHA1FromHexString("c5480468631a208ec8570b1bf35269a1f06a6001"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);

        // "The Hokuto Group asked us for a favor of sorts."
        // "Pops was into it since he'd nowe us big time for this one."
        // extra 'n' aside, this is also weird because the 'he' is referring to Seijuuro, which is
        // only apparent if you're with Mitsuki in this conversation. so make that a bit more
        // explicit.
        {
            static constexpr size_t offset = 0x21c3c;
            static constexpr size_t size = 0x86;
            std::vector<char> tmp;
            tmp.resize(size);
            std::memcpy(tmp.data(), &bin[offset], size);

            // remove the n
            tmp.erase(tmp.begin() + 0x66);

            // move linebreak
            std::swap(tmp[0x28], tmp[0x21]);

            // add the explicit reference to the chairman
            static constexpr auto chairman = STR_SPAN("'s chairman");
            tmp.insert(tmp.begin() + 0x1b, chairman.begin(), chairman.end());

            patcher.ReplaceCommand(offset, size, tmp);
        }

        // linebreaks
        bin[0x306a4] = 0x01;
        std::swap(bin[0x30ff7], bin[0x31001]);
        bin[0x32257] = 0x01;

        // "Speakin' of, any reason you're\x01hanging around our offices?"
        // This is written to a follow-up to a part of the conversation you only get if you've
        // previously seen Gotou at the shrine, so the 'speaking of' makes no sense if you don't get
        // that part. Remove that, the line works fine without it.
        std::swap(bin[0x30374], bin[0x3037c]);
        patcher.ReplacePartialCommand(0x30353, 0x3f, 0x30356, 0xe, {{0x41}});

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t6000
