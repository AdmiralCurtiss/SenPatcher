﻿#include <string_view>
#include <vector>

#include "p3a/pack.h"
#include "p3a/structs.h"
#include "sen/decompress_helper.h"
#include "sen/file_getter.h"
#include "util/bps.h"
#include "util/hash/sha1.h"

extern "C" {
__declspec(dllexport) char SenPatcherFix_1_t0250[] =
    "Fix a lot of dialogue issues in Pool/Training Ground.";
}

namespace {
static constexpr char PatchData[] = {
#include "embed_sen3_t0250_dat.h"
};
static constexpr size_t PatchLength = sizeof(PatchData);
} // namespace

namespace SenLib::Sen3::FileFixes::t0250_dat {
bool TryApply(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
              std::vector<SenPatcher::P3APackFile>& result) {
    try {
        auto file = getCheckedFile(
            "data/scripts/scena/dat_en/t0250.dat",
            224041,
            HyoutaUtils::Hash::SHA1FromHexString("1ba8784695ed86309b72a3104c7b0b81b67b503b"));
        if (!file) {
            return false;
        }
        auto patchdata = SenLib::DecompressFromBuffer(PatchData, PatchLength);
        if (!patchdata) {
            return false;
        }

        auto& bin = file->Data;

        // This file has a TON of changes, which makes patching them in-code very annoying and
        // error-prone. I suspect what happened here is that the PC version accidentally used the
        // on-disc PS4 script instead of the 1.02 patch like all the other files. So to save us some
        // pain we'll just patch it directly.

        HyoutaUtils::Stream::DuplicatableByteArrayStream source(bin.data(), bin.size());
        HyoutaUtils::Stream::DuplicatableByteArrayStream patch(patchdata->data(),
                                                               patchdata->size());
        std::vector<char> target;
        HyoutaUtils::Bps::ApplyPatchToStream(source, patch, target);

        result.emplace_back(std::move(target), file->Filename, SenPatcher::P3ACompressionType::LZ4);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::Sen3::FileFixes::t0250_dat
