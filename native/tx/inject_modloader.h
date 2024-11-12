#pragma once

#include <cstddef>
#include <cstdint>

#include "exe_patch.h"

namespace HyoutaUtils {
struct Logger;
}

namespace SenLib::TX {
enum class GameVersion;

struct PrFileHelperStruct {
    void* DataBuffer;
    uint32_t BufferSize;
};
static_assert(sizeof(PrFileHelperStruct) == 0x8);
static_assert(offsetof(PrFileHelperStruct, DataBuffer) == 0);
static_assert(offsetof(PrFileHelperStruct, BufferSize) == 4);

void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder);
void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder);
void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder);
void InjectAtPrFileOpen(PatchExecData& execData, void* prFileOpenForwarder);
void InjectAtPrFileExists(PatchExecData& execData, void* ffileGetFilesizeForwarder);
void InjectAtPrFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder);
} // namespace SenLib::TX
