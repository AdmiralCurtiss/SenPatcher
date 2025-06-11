#include "daybreak_save_convert.h"
#include "daybreak_save_convert_main.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "sentools/save_checksum_fix/save_checksum_fix.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace SenTools {
int Daybreak_Save_Convert_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "FILENAME",
        .Description = "The output directory to write the converted files to."};
    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_output}};
    static constexpr HyoutaUtils::Args args("sentools " Daybreak_Save_Convert_Name,
                                            "save000",
                                            Daybreak_Save_Convert_ShortDescription,
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
               options.FreeArguments.size() == 0 ? "No input directory given."
                                                 : "More than 1 input directory given.");
        args.PrintUsage();
        return -1;
    }

    std::string_view source(options.FreeArguments[0]);
    std::string_view target;
    if (auto* output_option = options.TryGetString(&arg_output)) {
        target = std::string_view(*output_option);
    } else {
        printf("Argument error: %s\n\n\n", "No output directory given.");
        args.PrintUsage();
        return -1;
    }

    auto result = DaybreakSaveConvert(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<DaybreakSaveConvertResult, std::string>
    DaybreakSaveConvert(std::string_view source, std::string_view target) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    static constexpr size_t PCUserLength = 0x1903b0;
    static constexpr size_t SwitchDetailLength = 1320;

    std::string sourceUserPath = std::string(source) + "/user.dat";
    HyoutaUtils::IO::File sourceUser(std::string_view(sourceUserPath),
                                     HyoutaUtils::IO::OpenMode::Read);
    if (!sourceUser.IsOpen()) {
        return std::string("Could not open " + sourceUserPath);
    }
    auto sourceUserLength = sourceUser.GetLength();
    if (!sourceUserLength) {
        return std::string("Read error: " + sourceUserPath);
    }
    if (*sourceUserLength > PCUserLength) {
        return std::string("File too long: " + sourceUserPath);
    }
    std::string sourceIconPath = std::string(source) + "/icon.png";
    HyoutaUtils::IO::File sourceIcon(std::string_view(sourceIconPath),
                                     HyoutaUtils::IO::OpenMode::Read);
    if (!sourceIcon.IsOpen()) {
        return std::string("Could not open " + sourceIconPath);
    }
    auto sourceIconLength = sourceIcon.GetLength();
    if (!sourceIconLength) {
        return std::string("Read error: " + sourceIconPath);
    }
    std::string sourceDetailPath = std::string(source) + "/detail";
    HyoutaUtils::IO::File sourceDetail(std::string_view(sourceDetailPath),
                                       HyoutaUtils::IO::OpenMode::Read);
    if (!sourceDetail.IsOpen()) {
        return std::string("Could not open " + sourceDetailPath);
    }
    auto sourceDetailLength = sourceDetail.GetLength();
    if (!sourceDetailLength) {
        return std::string("Read error: " + sourceDetailPath);
    }
    if (*sourceDetailLength != SwitchDetailLength) {
        return std::string("Wrong length: " + sourceDetailPath);
    }

    auto userBuffer = std::make_unique<char[]>(PCUserLength); // pad file to PC save length
    auto iconBuffer = std::make_unique<char[]>(*sourceIconLength);
    auto detailBuffer = std::make_unique<char[]>(*sourceDetailLength);
    if (sourceUser.Read(userBuffer.get(), *sourceUserLength) != *sourceUserLength) {
        return std::string("Read error: " + sourceUserPath);
    }
    if (sourceIcon.Read(iconBuffer.get(), *sourceIconLength) != *sourceIconLength) {
        return std::string("Read error: " + sourceIconPath);
    }
    if (sourceDetail.Read(detailBuffer.get(), *sourceDetailLength) != *sourceDetailLength) {
        return std::string("Read error: " + sourceDetailPath);
    }

    std::string targetDetailJsonString;
    {
        rapidjson::StringBuffer jsonbuffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
        json.StartObject();
        json.Key("Time");
        json.StartObject();
        json.Key("year");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x510), LittleEndian));
        json.Key("month");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x514), LittleEndian));
        json.Key("day");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x518), LittleEndian));
        json.Key("hour");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x51c), LittleEndian));
        json.Key("minute");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x520), LittleEndian));
        json.Key("second");
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x524), LittleEndian));
        json.EndObject();

        std::string_view title =
            HyoutaUtils::TextUtils::StripToNull(std::string_view(detailBuffer.get(), 0x84));
        std::string_view subtitle =
            HyoutaUtils::TextUtils::StripToNull(std::string_view(detailBuffer.get() + 0x84, 0x84));
        std::string_view description = HyoutaUtils::TextUtils::StripToNull(
            std::string_view(detailBuffer.get() + 0x108, 0x404));

        json.Key("title");
        json.String(title.data(), title.size());
        json.Key("subtitle");
        json.String(subtitle.data(), subtitle.size());
        json.Key("detail");
        json.String(description.data(), description.size());
        json.Key("user_param");
        // TODO: is this right?
        json.Uint(FromEndian(ReadUInt32(detailBuffer.get() + 0x50c), LittleEndian));
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
        SenTools::SaveChecksumFix(userBuffer.get(),
                                  PCUserLength,
                                  targetUserPath,
                                  SenTools::SaveFileType::Daybreak_GameData,
                                  true,
                                  true);
    if (saveChecksumFixResult.IsError()) {
        return saveChecksumFixResult.GetErrorValue();
    }
    if (!HyoutaUtils::IO::WriteFileAtomic(
            std::string_view(targetIconPath), iconBuffer.get(), *sourceIconLength)) {
        return std::string("Write error: " + targetIconPath);
    }
    if (!HyoutaUtils::IO::WriteFileAtomic(std::string_view(targetDetailPath),
                                          targetDetailJsonString.data(),
                                          targetDetailJsonString.size())) {
        return std::string("Write error: " + targetDetailPath);
    }

    return DaybreakSaveConvertResult::Success;
}
} // namespace SenTools
