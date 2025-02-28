#pragma once

#include "exe_patch.h"

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::Sen4 {
enum class GameVersion;

void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder);
void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder);
void InjectAtOpenFSoundFile(PatchExecData& execData, void* fsoundOpenForwarder);
void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder);
} // namespace SenLib::Sen4
