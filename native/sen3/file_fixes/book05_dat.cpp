#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sen/file_getter.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_4_book05[] = "Fix typos in Imperial Chronicle.";
}

namespace SenLib::Sen3::FileFixes::book05_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_en/book05.dat",
            40305,
            HyoutaUtils::Hash::SHA1FromHexString("b2b35b9d531658a45ed2a53477757d5a72c066ab"));
        if (!file) {
            return false;
        }

        HyoutaUtils::Stream::DuplicatableByteArrayStream bin(file->Data.data(), file->Data.size());
        BookTable book(bin, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Provincal -> Provincial
        auto& entry3 = book.Entries.at(3).Text.value();
        entry3.insert(entry3.begin() + 514, 'i');

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
} // namespace SenLib::Sen3::FileFixes::book05_dat
