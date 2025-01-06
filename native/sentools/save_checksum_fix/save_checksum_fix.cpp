#include "save_checksum_fix.h"
#include "save_checksum_fix_main.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "util/endian.h"
#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/scope.h"
#include "zstd/zstd.h"

namespace SenTools {
int Save_Checksum_Fix_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(Save_Checksum_Fix_ShortDescription);

    parser.usage("sentools " Save_Checksum_Fix_Name " [options] file.dat");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output file to write the fixed file to. Will fix file in-place if not given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
    } else {
        target = source;
    }

    auto result = SaveChecksumFix(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    if (result.GetSuccessValue() == SaveChecksumFixResult::AlreadyCorrect) {
        printf("%s\n", "This file already has a correct checksum.");
    }

    return 0;
}

namespace {
enum class SaveFileType {
    Unknown,
    CS4_SystemData,
    CS4_GameData,
    Reverie_SystemData,
    Reverie_GameData,
};
}

HyoutaUtils::Result<SaveChecksumFixResult, std::string> SaveChecksumFix(std::string_view source,
                                                                        std::string_view target) {
    using namespace HyoutaUtils::MemRead;
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::FromEndian;
    using HyoutaUtils::EndianUtils::ToEndian;
    using HyoutaUtils::EndianUtils::Endianness::BigEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    HyoutaUtils::IO::File infile(std::string_view(source), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        return std::string("Failed to get size of input file.");
    }
    if (*filesize < 4) {
        return std::string("File too small to be a CS/Reverie/TX save file.");
    }
    if (*filesize > (2u * 1024u * 1024u)) {
        return std::string("File too big to be a CS/Reverie/TX save file.");
    }
    size_t szlen = static_cast<size_t>(*filesize);

    auto saveMemory = std::make_unique_for_overwrite<char[]>(szlen);
    if (!saveMemory) {
        return std::string("Failed to allocate memory.");
    }
    if (infile.Read(saveMemory.get(), szlen) != szlen) {
        return std::string("Failed read input file.");
    }
    infile.Close();

    const uint32_t zstdIdent = FromEndian(ReadUInt32(saveMemory.get()), LittleEndian);
    bool wasCompressed = false;
    if (zstdIdent == 0xfd2fb528u) {
        // ZSTD compressed, decompress first
        const auto uncompressedDataLength = ZSTD_getFrameContentSize(saveMemory.get(), szlen);
        size_t decompressBufferSize = 0;
        if (uncompressedDataLength == ZSTD_CONTENTSIZE_UNKNOWN) {
            decompressBufferSize = (2u * 1024u * 1024u);
        } else if (uncompressedDataLength == ZSTD_CONTENTSIZE_ERROR) {
            return std::string("Failed to decompress input file.");
        } else if (uncompressedDataLength > (2u * 1024u * 1024u)) {
            return std::string("Decompressed file too big to be a CS/Reverie/TX save file.");
        } else {
            decompressBufferSize = static_cast<size_t>(uncompressedDataLength);
        }
        auto uncompressedDataBuffer = std::make_unique_for_overwrite<char[]>(decompressBufferSize);
        const size_t zstdReturn = ZSTD_decompress(
            uncompressedDataBuffer.get(), decompressBufferSize, saveMemory.get(), szlen);
        if (ZSTD_isError(zstdReturn)) {
            return std::string("Failed to decompress input file.");
        }
        saveMemory.swap(uncompressedDataBuffer);
        szlen = zstdReturn;
        wasCompressed = true;
    }

    if (szlen < 16) {
        return std::string("File too small to be a CS/Reverie/TX save file.");
    }

    // identify file
    SaveFileType saveFileType = SaveFileType::Unknown;
    const uint32_t header1 = FromEndian(ReadUInt32(saveMemory.get()), LittleEndian);
    const uint32_t header2 = FromEndian(ReadUInt32(saveMemory.get() + 4), LittleEndian);
    const uint32_t header3 = FromEndian(ReadUInt32(saveMemory.get() + 8), LittleEndian);
    const uint32_t header4 = FromEndian(ReadUInt32(saveMemory.get() + 12), LittleEndian);
    if (szlen == 1184 && header3 == szlen && header1 == 1 && header2 == 2) {
        saveFileType = SaveFileType::CS4_SystemData;
    } else if (szlen == 1463976 && header3 == szlen && header1 == 3 && header2 == 2) {
        saveFileType = SaveFileType::CS4_GameData;
    } else if (szlen == 3488 && header3 == szlen && header1 == 1 && header2 == 7) {
        saveFileType = SaveFileType::Reverie_SystemData;
    } else if (szlen == 1720032 && header3 == szlen && header1 == 8 && header2 == 3) {
        saveFileType = SaveFileType::Reverie_GameData;
    }

    if (saveFileType == SaveFileType::Unknown) {
        return std::string("File does not appear to be a savefile with a checksum to fix.");
    }

    const uint32_t checksum = crc_update(header3 - 16, saveMemory.get() + 16, szlen - 16);
    if (checksum == header4) {
        return SaveChecksumFixResult::AlreadyCorrect;
    }

    WriteUInt32(saveMemory.get() + 12, ToEndian(checksum, LittleEndian));

    if (wasCompressed) {
        // try to recompress, but if it doesn't work whatever,
        // the game also reads uncompressed files fine
        size_t bound = ZSTD_compressBound(szlen);
        if (!ZSTD_isError(bound)) {
            auto compressedData = std::make_unique_for_overwrite<char[]>(bound);
            const size_t zstdReturn =
                ZSTD_compress(compressedData.get(), bound, saveMemory.get(), szlen, 22);
            if (!ZSTD_isError(zstdReturn)) {
                saveMemory.swap(compressedData);
                szlen = zstdReturn;
            }
        }
    }

    HyoutaUtils::IO::File outfile;
    if (!outfile.OpenWithTempFilename(target, HyoutaUtils::IO::OpenMode::Write)) {
        return std::string("Failed to open output file.");
    }
    auto outfileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfile.Delete(); });
    if (outfile.Write(saveMemory.get(), szlen) != szlen) {
        return std::string("Failed to write to output file.");
    }
    if (!outfile.Rename(target)) {
        return std::string("Failed to rename output file.");
    }
    outfileScope.Dispose();

    return SaveChecksumFixResult::Success;
}
} // namespace SenTools
