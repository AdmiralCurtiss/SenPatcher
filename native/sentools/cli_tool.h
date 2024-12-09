#pragma once

namespace SenTools {
using CliToolFunctionT = int (*)(int argc, char** argv);
struct CliTool {
    const char* Name;
    const char* ShortDescription;
    CliToolFunctionT Function;
    bool Hidden = false;
};
} // namespace SenTools
