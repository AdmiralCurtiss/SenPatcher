#include <string>
#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/memwrite.h"
#include "util/stream.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_2_book01[] = "Fix minor text issues in Carnelia.";
}

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book01_dat {
static std::string RemoveLineStartingWith(std::string_view text, std::string_view s) {
    size_t idx = text.find(s);
    size_t endidx = text.find("\\n", idx);
    std::string result;
    result += text.substr(0, idx);
    result += text.substr(endidx + 2);
    return result;
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book01.dat",
            71733,
            HyoutaUtils::Hash::SHA1FromHexString("4243329ec1dd127cbf68a7f68d8ce6042225e1eb"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::MemWrite;
        using namespace HyoutaUtils::TextUtils;

        // missing hyphen
        WriteUInt8(&bin[0x1866], 0x2d);

        HyoutaUtils::Stream::DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // fix the really ugly (and inconsistent with other books) position of the chapter title
        for (auto& e : book.Entries) {
            if (e.Text.has_value() && e.Name.ends_with("01")) {
                // this looks better but still kinda sketchy
                // e.Text = e.Text.Replace("#-570y", "#-570y#250x");
                // e.Text = e.Text.Replace("#-585y", "#-580y");

                // this matches the formatting of Red Moon Rose's titles
                // by removing the line that states the chapter number
                e.Text = RemoveLineStartingWith(*e.Text, "#-570y");
                e.Text = Replace(*e.Text, "#-585y", "#-540y");
            }
        }

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book01_dat
