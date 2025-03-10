#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_2_book02[] = "Fix minor text issues in Red Moon Rose.";
}

namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book02_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book02.dat",
            189917,
            HyoutaUtils::Hash::SHA1FromHexString("ec509088c67eb02b279e132894f68986591d41af"));
        if (!file) {
            return false;
        }

        auto& bin = file->Data;

        using namespace HyoutaUtils::TextUtils;

        HyoutaUtils::Stream::DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // fix double apostrophes
        book.Entries[73].Text = Remove(*book.Entries[73].Text, 497, 1);
        book.Entries[148].Text = Remove(*book.Entries[148].Text, 46, 1);

        // fix missing linebreak
        book.Entries[321].Text = InsertSubstring(*book.Entries[321].Text, 476, "\\n", 0, 2);

        // the diamond pseudo-chapter-break was clearly intended to span 3 lines instead of just 1,
        // all text is layouted as if that was the case, so let's fix that
        for (auto& e : book.Entries) {
            if (e.Text.has_value() && e.Text->find("\xe2\x97\x87") != std::string::npos) {
                auto split = Split(*e.Text, "\\n");
                for (size_t i = 0; i < split.size(); ++i) {
                    if (split[i].find("\xe2\x97\x87") != std::string::npos) {
                        if (i == 0) {
                            split.insert(split.begin() + 1, "");
                        } else {
                            split.insert(split.begin() + i + 1, "");
                            split.insert(split.begin() + i, "");
                        }
                        break;
                    }
                }
                e.Text = Join(split, "\\n");
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
} // namespace SenLib::Sen1::FileFixes::scripts_book_dat_us_book02_dat
