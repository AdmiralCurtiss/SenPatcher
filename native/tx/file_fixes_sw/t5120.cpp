#include <cstring>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/file_getter.h"
#include "sen/sen_script_patcher.h"
#include "tx/file_fixes_sw.h"
#include "util/hash/sha1.h"
#include "util/vector.h"

namespace SenLib::TX::FileFixesSw::t5120 {
std::string_view GetDescription() {
    return "Text fixes in Nanahoshi Mall 2F.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto fileSw = FindAlreadyPackedFile(
            result,
            "scripts/scena/dat/t5120.dat",
            159001,
            HyoutaUtils::Hash::SHA1FromHexString("91333c9b239f775ca81cdfa233d8a9c227ebd211"));
        if (!fileSw || !fileSw->HasVectorData()) {
            return false;
        }

        auto bin = fileSw->GetVectorData();
        SenScriptPatcher patcher(bin);
        using HyoutaUtils::Vector::ShiftData;

        // the magical alisa gatcha list of figures needs a few fixes
        {
            static constexpr size_t offsetSeries1 = 0x252c1;
            static constexpr size_t offsetSeries2 = 0x25198;
            static constexpr size_t sizeSeries1 = 0xa1;
            static constexpr size_t sizeSeries2 = 0x109;
            std::vector<char> onlySeries1;
            onlySeries1.resize(sizeSeries1);
            std::vector<char> withSeries2;
            withSeries2.resize(sizeSeries2);
            std::memcpy(onlySeries1.data(), &bin[offsetSeries1], sizeSeries1);
            std::memcpy(withSeries2.data(), &bin[offsetSeries2], sizeSeries2);

            // make the headers consistent with itself and the Kagemaru set
            onlySeries1[0x1f] = 0x01;
            withSeries2[0x1f] = 0x01;
            ShiftData(onlySeries1, 0x17, 0x29, 9);
            ShiftData(withSeries2, 0x17, 0x29, 9);
            onlySeries1[0x1d] = 0x3a;
            withSeries2[0x1d] = 0x3a;
            onlySeries1.insert(onlySeries1.begin() + 0x28, (char)0x3a);
            withSeries2.insert(withSeries2.begin() + 0x28, (char)0x3a);
            withSeries2.erase(withSeries2.begin() + 0xa9, withSeries2.begin() + 0xaf);
            withSeries2[0xa9] = 0x3a;

            // list misspells Sara as Sarah
            onlySeries1.erase(onlySeries1.begin() + 0x66);
            withSeries2.erase(withSeries2.begin() + 0x66);


            // apply
            patcher.ReplaceCommand(offsetSeries1, sizeSeries1, onlySeries1);
            patcher.ReplaceCommand(offsetSeries2, sizeSeries2, withSeries2);
        }

        // double period after 'Obtained X' when getting the 2nd series secret figure
        patcher.RemovePartialCommand(0x24c11, 0x20, 0x24c2d, 0x1);

        // "Five base figures and a secret one..."
        // wrong, it's five in total, one of which is secret. just change to four + secret...
        bin[0x24133] = 0x6f;
        bin[0x24134] = 0x75;
        bin[0x24135] = 0x72;

        // "There's five base figures and a secret one."
        // same thing here
        bin[0x240cd] = 0x6f;
        bin[0x240ce] = 0x75;
        bin[0x240cf] = 0x72;

        // backslashes instead of newlines
        bin[0x677a] = 0x01;
        bin[0x1bf2e] = 0x01;

        // "#500W#3C#1P#3CHow frustrating...#20W\x01#300WHow sad...#15W\x01#600W...How fun."
        // patcher.ReplacePartialCommand(0x160e2, 0x53, 0x16123, 0x10, "");

        fileSw->SetVectorData(std::move(bin));
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX::FileFixesSw::t5120
