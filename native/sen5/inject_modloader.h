#pragma once

#include "exe_patch.h"

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::Sen5 {
enum class GameVersion;

void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder);
void InjectAtBattleScriptExists(PatchExecData& execData, void* ffileExistsForwarder);
void InjectAtFileExists1(PatchExecData& execData, void* ffileExistsForwarder);
void InjectAtFileExists2(PatchExecData& execData, void* ffileExistsForwarder);
void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder);
void InjectAtOpenFSoundFile(PatchExecData& execData, void* fsoundOpenForwarder);
} // namespace SenLib::Sen5
