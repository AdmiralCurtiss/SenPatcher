#include <filesystem>
#include <string_view>
#include <vector>

#include "file_getter.h"

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/book_table.h"
#include "sha1.h"
#include "util/stream.h"

namespace SenLib::Sen3::FileFixes::book05_dat {
std::string_view GetDescription() {
    return "Fix typos in Imperial Chronicle.";
}

bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/book/dat_en/book05.dat",
            40305,
            SenPatcher::SHA1FromHexString("b2b35b9d531658a45ed2a53477757d5a72c066ab"));
        if (!file) {
            return false;
        }

        DuplicatableByteArrayStream bin(file->Data.data(), file->Data.size());
        BookTable book(bin, HyoutaUtils::EndianUtils::Endianness::LittleEndian);

        // Provincal -> Provincial
        auto& entry3 = book.Entries.at(3).Text.value();
        entry3.insert(entry3.begin() + 514, 'i');

        std::vector<char> bin2;
        {
            MemoryStream ms(bin2);
            book.WriteToStream(ms, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
        }

        result.emplace_back(SenPatcher::P3APackFile{
            std::move(bin2), file->Filename, SenPatcher::P3ACompressionType::LZ4});

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::book05_dat
