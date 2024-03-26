#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sha1.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book05_dat {
std::string_view GetDescription() {
    return "Fix several consistency issues between encoded and decoded Black Records.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_us/book05.dat",
            56133,
            SenPatcher::SHA1FromHexString("dcd1d5c83e8201c42492abbe664f61ee3a2187ff"));
        if (!file) {
            return false;
        }

        using namespace HyoutaUtils::TextUtils;

        auto& bin = file->Data;
        DuplicatableByteArrayStream s(bin.data(), bin.size());
        BookTable book(s, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // clang-format off
        // completely missing line
        // this was fixed in CS2 PS4 (but didn't propagate into CS3, oops)
        book.Entries[4].Text = Insert(*book.Entries[4].Text, 59, "scars on the land,\\n \\n");

        // whitespace changes so that the encoded variants match
        book.Entries[2].Text = Remove(*book.Entries[2].Text, book.Entries[2].Text->size() - 3, 3);
        book.Entries[8].Text = Remove(*book.Entries[8].Text, book.Entries[8].Text->size() - 2, 2);
        book.Entries[21].Text = Remove(*book.Entries[21].Text, 241, 1);
        book.Entries[23].Text = ReplaceSubstring(*book.Entries[23].Text, 98, 2, *book.Entries[23].Text, 0, 1);
        book.Entries[23].Text = ReplaceSubstring(*book.Entries[23].Text, 103, 1, *book.Entries[23].Text, 52, 2);

        // missing paragraph start
        book.Entries[21].Text = InsertSubstring(*book.Entries[21].Text, 428, *book.Entries[21].Text, 0, 3);
        book.Entries[21].Text = ReplaceSubstring(*book.Entries[21].Text, 481, 1, *book.Entries[21].Text, 55, 2);
        book.Entries[21].Text = ReplaceSubstring(*book.Entries[21].Text, 486, 2, *book.Entries[21].Text, 0, 1);
        book.Entries[21].Text = ReplaceSubstring(*book.Entries[21].Text, 537, 1, *book.Entries[21].Text, 55, 2);
        book.Entries[21].Text = ReplaceSubstring(*book.Entries[21].Text, 541, 2, *book.Entries[21].Text, 0, 1);

        // sync the encoded variants
        book.Entries[63].Text = Remove(*book.Entries[63].Text, 424, 4);
        book.Entries[32].Text = book.Entries[63].Text;
        book.Entries[34].Text = InsertSubstring(*book.Entries[34].Text, 411, *book.Entries[34].Text, 411, 3);
        book.Entries[65].Text = book.Entries[34].Text;
        book.Entries[35].Text = InsertSubstring(*book.Entries[35].Text, 29, *book.Entries[35].Text, 147, 16);
        book.Entries[35].Text = InsertSubstring(*book.Entries[35].Text, 53, *book.Entries[35].Text, 148, 6);
        book.Entries[35].Text = ReplaceSubstring(*book.Entries[35].Text, 84, 1, *book.Entries[35].Text, 115, 5);
        book.Entries[35].Text = InsertSubstring(*book.Entries[35].Text, 50, *book.Entries[35].Text, 59, 13);
        book.Entries[35].Text = InsertSubstring(*book.Entries[35].Text, 50, *book.Entries[35].Text, 72, 16);
        book.Entries[35].Text = InsertSubstring(*book.Entries[35].Text, 50, *book.Entries[35].Text, 88, 7);
        book.Entries[66].Text = book.Entries[35].Text;
        book.Entries[39].Text = Remove(*book.Entries[39].Text, 429, 4);
        book.Entries[68].Text = ReplaceSubstring(*book.Entries[68].Text, 977, 7, *book.Entries[68].Text, 371, 1);
        book.Entries[37].Text = book.Entries[68].Text;
        book.Entries[70].Text = book.Entries[39].Text;
        book.Entries[49].Text = Remove(*book.Entries[49].Text, 850, 1);
        book.Entries[80].Text = book.Entries[49].Text;
        book.Entries[52].Text = Remove(*book.Entries[52].Text, 713, 11);
        book.Entries[52].Text = InsertSubstring(*book.Entries[52].Text, 716, *book.Entries[52].Text, 0, 3);
        book.Entries[52].Text = ReplaceSubstring(*book.Entries[52].Text, 803, 1, *book.Entries[52].Text, 89, 2);
        book.Entries[52].Text = ReplaceSubstring(*book.Entries[52].Text, 808, 2, *book.Entries[52].Text, 0, 1);
        book.Entries[52].Text = ReplaceSubstring(*book.Entries[52].Text, 897, 1, *book.Entries[52].Text, 89, 2);
        book.Entries[52].Text = ReplaceSubstring(*book.Entries[52].Text, 901, 2, *book.Entries[52].Text, 0, 1);
        book.Entries[83].Text = book.Entries[52].Text;
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 45, 1, *book.Entries[23].Text, 23, 7);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 91, 13, *book.Entries[23].Text, 51, 54);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 46, 6, *book.Entries[85].Text, 145, 18);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 114, 7, *book.Entries[85].Text, 157, 21);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 137, 3, *book.Entries[85].Text, 171, 9);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 147, 4, *book.Entries[85].Text, 177, 12);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 161, 3, *book.Entries[85].Text, 185, 9);
        book.Entries[85].Text = ReplaceSubstring(*book.Entries[85].Text, 171, 7, *book.Entries[85].Text, 191, 21);
        book.Entries[54].Text = book.Entries[85].Text;
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 367, 3, *book.Entries[7].Text, 213, 7);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 407, 165, *book.Entries[7].Text, 237, 100);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 411, 3, *book.Entries[38].Text, 158, 9);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 421, 4, *book.Entries[38].Text, 158, 12);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 457, 9, *book.Entries[38].Text, 158, 27);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 492, 8, *book.Entries[38].Text, 158, 24);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 531, 5, *book.Entries[38].Text, 158, 15);
        book.Entries[38].Text = ReplaceSubstring(*book.Entries[38].Text, 551, 6, *book.Entries[38].Text, 158, 18);
        book.Entries[69].Text = book.Entries[38].Text;

        /*
        void RemapOffset(std::string_view a, size_t offset, size_t length) {
            size_t inoffset = 0;
            size_t inlengthoffset = 0;
            for (size_t i = 0; i < a.size(); ++i) {
                if (inoffset == offset) {
                    inlengthoffset = i;
                    if (offset != i)
                        printf("offset %d remaps to %d\n", (int)offset, (int)i);
                }
                if (inoffset == offset + length) {
                    if (length != (i - inlengthoffset))
                        printf("length %d remaps to %d\n", (int)length, (int)(i - inlengthoffset));
                    return;
                }

                char c = a[i];
                ++inoffset;
                if ((c & 0b1110'0000) == 0b1100'0000) {
                    ++i;
                } else if ((c & 0b1111'0000) == 0b1110'0000) {
                    ++i;
                    ++i;
                }
            }

            if (length != (a.size() - inlengthoffset))
                printf("length %d remaps to %d\n", (int)length, (int)(a.size() - inlengthoffset));
            return;
        }


        for (int i = 32; i < 62; ++i) {
            if (!(book.Entries[i].Text == book.Entries[i + 31].Text)) {
                System.Console.WriteLine("mismatch between {0} ({1}) and {2} ({3})", book.Entries[i].Name, i, book.Entries[i + 31].Name, i + 31);
            }
        }
        for (int i = 1; i < 31; ++i) {
            string decoded = book.Entries[i].Text;
            string encoded = book.Entries[i + 31].Text;
            bool same = false;
            if (decoded == null && encoded == null) {
                same = true;
            } else if (decoded.Length == encoded.Length) {
                same = true;
                for (int j = 0; j < decoded.Length; ++j) {
                    char dc = decoded[j];
                    char ec = encoded[j];
                    if (ec == '\x2503') {
                        if (!((dc >= 'A' && dc <= 'Z') || (dc >= 'a' && dc <= 'z') || (dc >= '0' && dc <= '9') || dc == '\'' || dc == '?')) {
                            same = false;
                            break;
                        }
                    } else if (dc != ec) {
                        same = false;
                        break;
                    }
                }
            }
            if (!same) {
                System.Console.WriteLine("mismatch between decoded {0} ({1}) and encoded {2} ({3})", book.Entries[i].Name, i, book.Entries[i + 31].Name, i + 31);
            }
        }
        //*/
        // clang-format on

        std::vector<char> bin2;
        MemoryStream ms(bin2);
        book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen2::FileFixes::scripts_book_dat_us_book05_dat
