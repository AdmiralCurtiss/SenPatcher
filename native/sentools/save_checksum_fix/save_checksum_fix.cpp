#include "save_checksum_fix.h"
#include "save_checksum_fix_main.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/crc32.h"
#include "util/memread.h"
#include "util/memwrite.h"
#include "util/scope.h"
#include "zstd/zstd.h"

namespace SenTools {
int Save_Checksum_Fix_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "FILENAME",
        .Description =
            "The output file to write the fixed file to. Will fix file in-place if not given."};
    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_output}};
    static constexpr HyoutaUtils::Args args("sentools " Save_Checksum_Fix_Name,
                                            "file.bin",
                                            Save_Checksum_Fix_ShortDescription,
                                            args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input file given."
                                                 : "More than 1 input file given.");
        args.PrintUsage();
        return -1;
    }

    std::string_view source(options.FreeArguments[0]);
    std::string_view target;
    if (auto* output_option = options.TryGetString(&arg_output)) {
        target = std::string_view(*output_option);
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

HyoutaUtils::Result<SaveChecksumFixResult, std::string>
    SaveChecksumFix(std::string_view source,
                    std::string_view target,
                    SaveFileType saveFileTypeInput,
                    bool forceCompress,
                    bool forceWrite) {
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

    return SaveChecksumFix(
        saveMemory.get(), szlen, target, saveFileTypeInput, forceCompress, forceWrite);
}

HyoutaUtils::Result<SaveChecksumFixResult, std::string>
    SaveChecksumFix(char* saveMemoryData,
                    size_t saveMemoryLength,
                    std::string_view target,
                    SaveFileType saveFileTypeInput,
                    bool forceCompress,
                    bool forceWrite) {
    using namespace HyoutaUtils::MemRead;
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::FromEndian;
    using HyoutaUtils::EndianUtils::ToEndian;
    using HyoutaUtils::EndianUtils::Endianness::BigEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    char* saveMemory = saveMemoryData;
    size_t szlen = saveMemoryLength;

    std::unique_ptr<char[]> uncompressedDataBuffer;
    const uint32_t zstdIdent = FromEndian(ReadUInt32(saveMemory), LittleEndian);
    bool wasCompressed = false;
    if (zstdIdent == 0xfd2fb528u) {
        // ZSTD compressed, decompress first
        const auto uncompressedDataLength = ZSTD_getFrameContentSize(saveMemory, szlen);
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
        uncompressedDataBuffer = std::make_unique_for_overwrite<char[]>(decompressBufferSize);
        const size_t zstdReturn =
            ZSTD_decompress(uncompressedDataBuffer.get(), decompressBufferSize, saveMemory, szlen);
        if (ZSTD_isError(zstdReturn)) {
            return std::string("Failed to decompress input file.");
        }
        saveMemory = uncompressedDataBuffer.get();
        szlen = zstdReturn;
        wasCompressed = true;
    }

    if (szlen < 16) {
        return std::string("File too small to be a CS/Reverie/TX save file.");
    }

    // identify file
    SaveFileType saveFileType = saveFileTypeInput;
    const uint32_t header1 = FromEndian(ReadUInt32(saveMemory), LittleEndian);
    const uint32_t header2 = FromEndian(ReadUInt32(saveMemory + 4), LittleEndian);
    const uint32_t header3 = FromEndian(ReadUInt32(saveMemory + 8), LittleEndian);
    const uint32_t header4 = FromEndian(ReadUInt32(saveMemory + 12), LittleEndian);
    if (saveFileType == SaveFileType::Unknown) {
        // try to autodetect
        if (szlen == 1184 && header3 == szlen && header1 == 1 && header2 == 2) {
            saveFileType = SaveFileType::CS4_SystemData;
        } else if (szlen == 1463976 && header3 == szlen && header1 == 3 && header2 == 2) {
            saveFileType = SaveFileType::CS4_GameData;
        } else if (szlen == 3488 && header3 == szlen && header1 == 1 && header2 == 7) {
            saveFileType = SaveFileType::Reverie_SystemData;
        } else if (szlen == 1720032 && header3 == szlen && header1 == 8 && header2 == 3) {
            saveFileType = SaveFileType::Reverie_GameData;
        }
    }

    uint32_t initialCrcValue;
    size_t sumDataStartOffset;
    size_t sumDataLength;
    size_t sumOffset;
    switch (saveFileType) {
        case SaveFileType::CS4_SystemData:
        case SaveFileType::CS4_GameData:
        case SaveFileType::Reverie_SystemData:
        case SaveFileType::Reverie_GameData:
            initialCrcValue = header3 - 16;
            sumDataStartOffset = 16;
            sumDataLength = szlen - 16;
            sumOffset = 12;
            break;
        case SaveFileType::Daybreak_GameData:
        case SaveFileType::Daybreak2_GameData:
            initialCrcValue = static_cast<uint32_t>(szlen - 12);
            sumDataStartOffset = 12;
            sumDataLength = szlen - 12;
            sumOffset = 8;
            break;
        default:
            return std::string("File does not appear to be a savefile with a checksum to fix.");
    }

    const uint32_t checksum =
        crc_update(initialCrcValue, saveMemory + sumDataStartOffset, sumDataLength);
    if (!forceWrite && checksum == FromEndian(ReadUInt32(saveMemory + sumOffset), LittleEndian)) {
        return SaveChecksumFixResult::AlreadyCorrect;
    }

    WriteUInt32(saveMemory + sumOffset, ToEndian(checksum, LittleEndian));

    std::unique_ptr<char[]> compressedData;
    if (wasCompressed || forceCompress) {
        // try to recompress, but if it doesn't work whatever,
        // the game also reads uncompressed files fine
        size_t bound = ZSTD_compressBound(szlen);
        if (!ZSTD_isError(bound)) {
            compressedData = std::make_unique_for_overwrite<char[]>(bound);
            const size_t zstdReturn =
                ZSTD_compress(compressedData.get(), bound, saveMemory, szlen, 22);
            if (!ZSTD_isError(zstdReturn)) {
                saveMemory = compressedData.get();
                szlen = zstdReturn;
            }
        }
    }

    HyoutaUtils::IO::File outfile;
    if (!outfile.OpenWithTempFilename(target, HyoutaUtils::IO::OpenMode::Write)) {
        return std::string("Failed to open output file.");
    }
    auto outfileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfile.Delete(); });
    if (outfile.Write(saveMemory, szlen) != szlen) {
        return std::string("Failed to write to output file.");
    }
    if (!outfile.Flush()) {
        return std::string("Failed to flush output file.");
    }
    if (!outfile.Rename(target)) {
        return std::string("Failed to rename output file.");
    }
    outfileScope.Dispose();

    return SaveChecksumFixResult::Success;
}
} // namespace SenTools
