#include <cstdio>
#include <exception>
#include <string_view>

#include "senpatcher_version.h"

#include "util/text.h"

#include "cli_tool.h"
#include "p3a_extract.h"
#include "p3a_pack.h"
#include "p3a_repack.h"
#include "pka_extract.h"
#include "pka_pack.h"
#include "pkg_extract.h"
#include "pkg_pack.h"
#include "pkg_repack.h"

namespace SenTools {
static constexpr auto CliTools = {
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
};
}

static void PrintUsage() {
    printf("SenTools from SenPatcher " SENPATCHER_VERSION "\n");
    printf("Select one of the following tools via the first argument:\n");
    for (const auto& tool : SenTools::CliTools) {
        printf(" %-15s %s\n", tool.Name, tool.ShortDescription);
    }
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            PrintUsage();
            return -1;
        }

        const std::string_view name = argv[1];
        for (const auto& tool : SenTools::CliTools) {
            if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(name, tool.Name)) {
                return tool.Function(argc - 1, argv + 1);
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
