#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_4_book06[] = "Fix typos in 'Other' books.";
}

namespace SenLib::Sen3::FileFixes::book06_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_en/book06.dat",
            18830,
            HyoutaUtils::Hash::SHA1FromHexString("fa7e69755a3592cdb5196cd45ac861671798384c"));
        if (!file) {
            return false;
        }

        HyoutaUtils::Stream::DuplicatableByteArrayStream bin(file->Data.data(), file->Data.size());
        BookTable book(bin, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // provincal -> provincial
        auto& entry24 = book.Entries.at(24).Text.value();
        entry24.insert(entry24.begin() + 93, 'i');

        std::vector<char> bin2;
        {
            HyoutaUtils::Stream::MemoryStream ms(bin2);
            book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        }

        result.emplace_back(std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::book06_dat
