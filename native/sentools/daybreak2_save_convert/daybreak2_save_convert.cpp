#include "daybreak2_save_convert.h"
#include "daybreak2_save_convert_main.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "sentools/save_checksum_fix/save_checksum_fix.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace SenTools {
int Daybreak2_Save_Convert_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(Daybreak2_Save_Convert_ShortDescription);

    parser.usage("sentools " Daybreak2_Save_Convert_Name " [options] save000");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output directory to write the converted files to.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
    } else {
        parser.error("No output directory given.");
        return -1;
    }

    auto result = Daybreak2SaveConvert(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<Daybreak2SaveConvertResult, std::string>
    Daybreak2SaveConvert(std::string_view source, std::string_view target) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    static constexpr size_t PCUserLength = 0x271918;

    std::string sourceUserPath = std::string(source) + "/data.dat";
    HyoutaUtils::IO::File sourceUser(std::string_view(sourceUserPath),
                                     HyoutaUtils::IO::OpenMode::Read);
    if (!sourceUser.IsOpen()) {
        return std::string("Could not open " + sourceUserPath);
    }
    auto sourceUserLength = sourceUser.GetLength();
    if (!sourceUserLength) {
        return std::string("Read error: " + sourceUserPath);
    }
    if (*sourceUserLength < 0x38) {
        return std::string("Input error: " + sourceUserPath + " too small");
    }
    auto inputBuffer = std::make_unique<char[]>(*sourceUserLength);
    if (sourceUser.Read(inputBuffer.get(), *sourceUserLength) != *sourceUserLength) {
        return std::string("Read error: " + sourceUserPath);
    }

    std::array<uint32_t, 7> offsets{};
    std::array<uint32_t, 7> sizes{};
    for (size_t i = 0; i < 7; ++i) {
        offsets[i] = FromEndian(ReadUInt32(&inputBuffer[i * 4]), LittleEndian);
        sizes[i] = FromEndian(ReadUInt32(&inputBuffer[0x1c + i * 4]), LittleEndian);
        if (offsets[i] > *sourceUserLength) {
            return std::string("Invalid offset in input file header");
        }
        if (sizes[i] > *sourceUserLength || offsets[i] > (*sourceUserLength - sizes[i])) {
            return std::string("Invalid size in input file header");
        }
    }

    // 0 -> user.dat
    // 1 -> icon0.png
    // 2 -> timestamps
    // 3 -> title
    // 4 -> subtitle
    // 5 -> detail
    // 6 -> user_param

    if (sizes[0] != PCUserLength) {
        return std::string("Invalid size for user.dat section");
    }
    if (sizes[2] != 0x1C) {
        return std::string("Invalid size for timestamp section");
    }
    if (sizes[6] != 0x4) {
        return std::string("Invalid size for user_param section");
    }

    std::string targetDetailJsonString;
    {
        rapidjson::StringBuffer jsonbuffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
        json.StartObject();
        json.Key("Time");
        json.StartObject();
        json.Key("year");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2]), LittleEndian));
        json.Key("month");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2] + 0x4), LittleEndian));
        json.Key("day");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2] + 0x8), LittleEndian));
        json.Key("hour");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2] + 0xc), LittleEndian));
        json.Key("minute");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2] + 0x10), LittleEndian));
        json.Key("second");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[2] + 0x14), LittleEndian));
        json.EndObject();

        std::string_view title = HyoutaUtils::TextUtils::StripToNull(
            std::string_view(inputBuffer.get() + offsets[3], sizes[3]));
        std::string_view subtitle = HyoutaUtils::TextUtils::StripToNull(
            std::string_view(inputBuffer.get() + offsets[4], sizes[4]));
        std::string_view description = HyoutaUtils::TextUtils::StripToNull(
            std::string_view(inputBuffer.get() + offsets[5], sizes[5]));

        json.Key("title");
        json.String(title.data(), title.size());
        json.Key("subtitle");
        json.String(subtitle.data(), subtitle.size());
        json.Key("detail");
        json.String(description.data(), description.size());
        json.Key("user_param");
        json.Uint(FromEndian(ReadUInt32(inputBuffer.get() + offsets[6]), LittleEndian));
        json.EndObject();

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        targetDetailJsonString.assign(jsonstring, jsonstringsize);
    }

    HyoutaUtils::IO::CreateDirectory(target);
    std::string targetUserPath = std::string(target) + "/user.dat";
    std::string targetIconPath = std::string(target) + "/icon0.png";
    std::string targetDetailPath = std::string(target) + "/detail.json";
    auto saveChecksumFixResult =
        SenTools::SaveChecksumFix(inputBuffer.get() + offsets[0],
                                  sizes[0],
                                  targetUserPath,
                                  SenTools::SaveFileType::Daybreak2_GameData,
                                  true,
                                  true);
    if (saveChecksumFixResult.IsError()) {
        return saveChecksumFixResult.GetErrorValue();
    }
    if (!HyoutaUtils::IO::WriteFileAtomic(
            std::string_view(targetIconPath), inputBuffer.get() + offsets[1], sizes[1])) {
        return std::string("Write error: " + targetIconPath);
    }
    if (!HyoutaUtils::IO::WriteFileAtomic(std::string_view(targetDetailPath),
                                          targetDetailJsonString.data(),
                                          targetDetailJsonString.size())) {
        return std::string("Write error: " + targetDetailPath);
    }

    return Daybreak2SaveConvertResult::Success;
}
} // namespace SenTools
