#include <span>
#include <string_view>
#include <vector>

#include "sen1/file_fixes/scripts_book_dat_us_book00_dat_shared.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"
#include "util/text.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_2_book00[] =
    "Fix consistency issues with CS1 in Imperial Chronicle back-issues.";
}

namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book00_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book00.dat",
            51128,
            HyoutaUtils::Hash::SHA1FromHexString("7a68747acbd132c5215ea9c7543c37f146b39d63"));
        if (!file) {
            return false;
        }

        using namespace HyoutaUtils::TextUtils;

        auto& bin = file->Data;
        HyoutaUtils::Stream::DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // weirdly formatted sub-headline
        book.Entries[3].Text = InsertSubstring(*book.Entries[3].Text, 37, "\xe2\x97\x86", 0, 3);
        book.Entries[3].Text = InsertSubstring(*book.Entries[3].Text, 5, "\xe2\x97\x86", 0, 3);
        book.Entries[3].Text =
            ReplaceSubstring(*book.Entries[3].Text, 0, 5, *book.Entries[2].Text, 0, 5);
        book.Entries[3].Text =
            ReplaceSubstring(*book.Entries[3].Text, 601, 89, *book.Entries[9].Text, 471, 89);

        // clean up whitespace for easier diffing
        Sen1::FileFixes::scripts_book_dat_us_book00_dat::CleanUpWhitespace(book);

        // consistency fixes to match CS1
        book.Entries[8].Text =
            ReplaceSubstring(*book.Entries[8].Text, 398, 5, *book.Entries[8].Text, 57, 5);
        book.Entries[39].Text =
            ReplaceSubstring(*book.Entries[39].Text, 313, 1, *book.Entries[39].Text, 72, 2);
        book.Entries[42].Text =
            ReplaceSubstring(*book.Entries[42].Text, 439, 3, *book.Entries[42].Text, 631, 2);
        book.Entries[81].Text =
            ReplaceSubstring(*book.Entries[81].Text, 355, 5, *book.Entries[81].Text, 55, 5);
        book.Entries[85].Text =
            InsertSubstring(*book.Entries[85].Text, 18, *book.Entries[85].Text, 94, 1);

        Sen1::FileFixes::scripts_book_dat_us_book00_dat::PostSyncFixes(book);

        std::vector<char> bin2;
        HyoutaUtils::Stream::MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book00_dat
