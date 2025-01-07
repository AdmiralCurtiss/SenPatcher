#include <cstdio>
#include <exception>
#include <memory>
#include <string_view>

#include "senpatcher_version.h"

#include "util/text.h"

#include "cli_tool.h"

#include "bra_extract/bra_extract_main.h"
#include "dirtree_create/dirtree_create_main.h"
#include "game_verify/game_verify_main.h"
#include "p3a_extract/p3a_extract_main.h"
#include "p3a_pack/p3a_pack_main.h"
#include "p3a_repack/p3a_repack_main.h"
#include "pka_extract/pka_extract_main.h"
#include "pka_pack/pka_pack_main.h"
#include "pkg_extract/pkg_extract_main.h"
#include "pkg_pack/pkg_pack_main.h"
#include "pkg_repack/pkg_repack_main.h"
#include "save_checksum_fix/save_checksum_fix_main.h"
#include "type1_compress/type1_compress_main.h"
#include "type1_decompress/type1_decompress_main.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef SENTOOLS_WITH_IMGUI
#include "gui/main_gui.h"
#endif

namespace SenTools {
static constexpr auto CliTools = {
#ifdef SENTOOLS_WITH_IMGUI
    CliTool{.Name = "GUI",
            .ShortDescription = "Start the interactive GUI.",
            .Function = SenTools::RunGui},
#endif
    CliTool{.Name = P3A_Extract_Name,
            .ShortDescription = P3A_Extract_ShortDescription,
            .Function = P3A_Extract_Function},
    CliTool{.Name = P3A_Pack_Name,
            .ShortDescription = P3A_Pack_ShortDescription,
            .Function = P3A_Pack_Function},
    CliTool{.Name = P3A_Repack_Name,
            .ShortDescription = P3A_Repack_ShortDescription,
            .Function = P3A_Repack_Function},
    CliTool{.Name = PKG_Extract_Name,
            .ShortDescription = PKG_Extract_ShortDescription,
            .Function = PKG_Extract_Function},
    CliTool{.Name = PKG_Pack_Name,
            .ShortDescription = PKG_Pack_ShortDescription,
            .Function = PKG_Pack_Function},
    CliTool{.Name = PKG_Repack_Name,
            .ShortDescription = PKG_Repack_ShortDescription,
            .Function = PKG_Repack_Function},
    CliTool{.Name = PKA_Extract_Name,
            .ShortDescription = PKA_Extract_ShortDescription,
            .Function = PKA_Extract_Function},
    CliTool{.Name = PKA_Pack_Name,
            .ShortDescription = PKA_Pack_ShortDescription,
            .Function = PKA_Pack_Function},
    CliTool{.Name = BRA_Extract_Name,
            .ShortDescription = BRA_Extract_ShortDescription,
            .Function = BRA_Extract_Function},
    CliTool{.Name = Type1_Decompress_Name,
            .ShortDescription = Type1_Decompress_ShortDescription,
            .Function = Type1_Decompress_Function},
    CliTool{.Name = Type1_Compress_Name,
            .ShortDescription = Type1_Compress_ShortDescription,
            .Function = Type1_Compress_Function},
    CliTool{.Name = Game_Verify_Name,
            .ShortDescription = Game_Verify_ShortDescription,
            .Function = Game_Verify_Function},
    CliTool{.Name = Save_Checksum_Fix_Name,
            .ShortDescription = Save_Checksum_Fix_ShortDescription,
            .Function = Save_Checksum_Fix_Function},
    CliTool{.Name = DirTree_Create_Name,
            .ShortDescription = DirTree_Create_ShortDescription,
            .Function = DirTree_Create_Function,
            .Hidden = true},
};
} // namespace SenTools

static void PrintUsage() {
    printf("SenTools from SenPatcher " SENPATCHER_VERSION "\n");
    printf("Select one of the following tools via the first argument:\n");
    for (const auto& tool : SenTools::CliTools) {
        if (!tool.Hidden) {
            printf(" %-18s %s\n", tool.Name, tool.ShortDescription);
        }
    }
}

#ifdef BUILD_FOR_WINDOWS
#ifndef SENTOOLS_WITH_IMGUI
static const char* StripPathToFilename(const char* path) {
    const char* tmp = path;
    const char* rv = path;
    while (*tmp != '\0') {
        if (*tmp == '/' || *tmp == '\\') {
            rv = tmp + 1;
        }
        ++tmp;
    }
    return rv;
}
#endif
#endif

static int InternalMain(int argc, char** argvUtf8) {
    try {
        if (argc < 2) {
#ifdef BUILD_FOR_WINDOWS
            // https://devblogs.microsoft.com/oldnewthing/20160125-00/?p=92922
            // Check to see if the user launched this by double-clicking so we can tell them that
            // this is a command line app -- you won't believe how many times I've seen someone be
            // confused by the 'flashing window' in discord...
            DWORD tmp;
            if (GetConsoleProcessList(&tmp, 1) == 1) {
#ifdef SENTOOLS_WITH_IMGUI
                return SenTools::RunGui(argc, argvUtf8);
#else
                PrintUsage();
                printf(
                    "\n"
                    "This is a command line application.\n"
                    "To use this, open a command prompt or PowerShell and type the name of this "
                    "executable (%s)\n"
                    "followed by the name of the sub-tool you'd like to use.\n"
                    "You will then get further instructions on what arguments are expected by the "
                    "sub-tool.\n"
                    "\n",
                    (argc >= 1 ? StripPathToFilename(argvUtf8[0]) : "sentools.exe"));
                system("pause");
#endif
            } else {
                PrintUsage();
            }
#else
            PrintUsage();
#endif

            return -1;
        }

        const std::string_view name = argvUtf8[1];
        for (const auto& tool : SenTools::CliTools) {
            if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(name, tool.Name)) {
                return tool.Function(argc - 1, argvUtf8 + 1);
            }
        }

        PrintUsage();
        return -1;
    } catch (const std::exception& ex) {
        printf("Exception occurred: %s\n", ex.what());
        return -3;
    } catch (...) {
        printf("Unknown error occurred.\n");
        return -4;
    }
}

#ifdef BUILD_FOR_WINDOWS
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
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
