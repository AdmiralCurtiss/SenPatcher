﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "sen3/file_fixes.h"
#include "sen3/patch_pkg.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_8_I_CVIS1008[] = "Fix Rontes nametag.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_cvis1008.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::I_CVIS1008_pkg {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/asset/D3D11_us/I_CVIS1008.pkg",
            44196,
            HyoutaUtils::Hash::SHA1FromHexString("3721996e74336af0133f096b1f717e83b9bdc075"));
        if (!file) {
            return false;
        }
        auto patch = DecompressFromBuffer(PatchData, PatchLength);
        if (!patch) {
            return false;
        }

        auto& bin = file->Data;
        auto tex = PatchSingleFileInPkg(bin.data(), bin.size(), patch->data(), patch->size(), 2, 1);

        result.emplace_back(std::move(tex), file->Filename, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::I_CVIS1008_pkg
