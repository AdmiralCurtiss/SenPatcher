#include "sen2/file_fixes.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "modload/loaded_mods.h"
#include "modload/loaded_pka.h"

#include "util/args.h"
#include "util/logger.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

static constexpr std::array<std::string_view, 5> s_VanillaP3ANames{
    {"misc.p3a", "bgm.p3a", "se.p3a", "voice.p3a", "voice_jp.p3a"}};
static constexpr std::array<std::string_view, 2> s_PkaGroupPrefixes{
    {"data/asset/d3d11/", "data/asset/d3d11_us/"}};

int InternalMain(int argc, char** argv) {
    static constexpr HyoutaUtils::Args args("cs2test",
                                            std::string_view(),
                                            std::string_view(),
                                            std::span<const HyoutaUtils::Arg* const>());
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

    std::string_view baseDirUtf8 = options.FreeArguments[0];
    HyoutaUtils::Logger logger;
    SenLib::ModLoad::LoadedP3AData vanillaP3As;
    SenLib::ModLoad::LoadedPkaData vanillaPKAs;
    LoadP3As(logger, vanillaP3As, baseDirUtf8, s_VanillaP3ANames);
    LoadPkas(logger, vanillaPKAs, baseDirUtf8, s_PkaGroupPrefixes, {});
    SenLib::ModLoad::CreateModDirectory(baseDirUtf8);
    SenLib::Sen2::CreateAssetPatchIfNeeded(
        logger, baseDirUtf8, vanillaP3As, vanillaPKAs, s_PkaGroupPrefixes);
    return 0;
}

#ifdef BUILD_FOR_WINDOWS
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
    // don't display any 'please insert a disk into drive x' textboxes
    // when accessing a drive with removable storage
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    SetConsoleOutputCP(65001); // we use printf() with utf8 strings

    std::unique_ptr<const char*[]> argvUtf8Pts;
    std::unique_ptr<std::string[]> argvStorage;
    const size_t argcSizet = static_cast<size_t>(argc > 0 ? argc : 0);
    argvUtf8Pts = std::make_unique<const char*[]>(argcSizet + 1);
    argvStorage = std::make_unique<std::string[]>(argcSizet);
    for (size_t i = 0; i < argcSizet; ++i) {
        auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(argv[i], wcslen(argv[i]));
        if (utf8) {
            argvStorage[i] = std::move(*utf8);
        } else {
            printf("Failed to convert argument %zu to UTF8.\n", i);
            return -1;
        }
        argvUtf8Pts[i] = argvStorage[i].c_str();
    }
    argvUtf8Pts[argcSizet] = nullptr;
    return InternalMain(argc, const_cast<char**>(argvUtf8Pts.get()));
}
#else
int main(int argc, char** argv) {
    return InternalMain(argc, argv);
}
#endif
