#pragma once

#include "exe_patch.h"

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::Sen2 {
enum class GameVersion;

void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder);
void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder);
} // namespace SenLib::Sen2
