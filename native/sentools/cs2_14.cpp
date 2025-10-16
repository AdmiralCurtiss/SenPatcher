#include "cs2_14.h"

#include <functional>
#include <span>
#include <string>
#include <string_view>

#include "pe_exe.h"
#include "sen/decompress_helper.h"
#include "util/bps.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

namespace {
static constexpr char PatchData_j140_to_j142_data[] = {
#include "embed_sen2_ed82_j140_to_j142_data.h"
};
static constexpr size_t PatchLength_j140_to_j142_data = sizeof(PatchData_j140_to_j142_data);
static constexpr char PatchData_j140_to_j142_header[] = {
#include "embed_sen2_ed82_j140_to_j142_header.h"
};
static constexpr size_t PatchLength_j140_to_j142_header = sizeof(PatchData_j140_to_j142_header);
static constexpr char PatchData_j140_to_j142_rdata[] = {
#include "embed_sen2_ed82_j140_to_j142_rdata.h"
};
static constexpr size_t PatchLength_j140_to_j142_rdata = sizeof(PatchData_j140_to_j142_rdata);
static constexpr char PatchData_j140_to_j142_rsrc[] = {
#include "embed_sen2_ed82_j140_to_j142_rsrc.h"
};
static constexpr size_t PatchLength_j140_to_j142_rsrc = sizeof(PatchData_j140_to_j142_rsrc);
static constexpr char PatchData_j140_to_j142_text[] = {
#include "embed_sen2_ed82_j140_to_j142_text.h"
};
static constexpr size_t PatchLength_j140_to_j142_text = sizeof(PatchData_j140_to_j142_text);
static constexpr char PatchData_j142_to_u142_data[] = {
#include "embed_sen2_ed82_j142_to_u142_data.h"
};
static constexpr size_t PatchLength_j142_to_u142_data = sizeof(PatchData_j142_to_u142_data);
static constexpr char PatchData_j142_to_u142_header[] = {
#include "embed_sen2_ed82_j142_to_u142_header.h"
};
static constexpr size_t PatchLength_j142_to_u142_header = sizeof(PatchData_j142_to_u142_header);
static constexpr char PatchData_j142_to_u142_rsrc[] = {
#include "embed_sen2_ed82_j142_to_u142_rsrc.h"
};
static constexpr size_t PatchLength_j142_to_u142_rsrc = sizeof(PatchData_j142_to_u142_rsrc);
static constexpr char PatchData_j142_to_u142_text[] = {
#include "embed_sen2_ed82_j142_to_u142_text.h"
};
static constexpr size_t PatchLength_j142_to_u142_text = sizeof(PatchData_j142_to_u142_text);
static constexpr char PatchData_u140j142_to_u142_rdata[] = {
#include "embed_sen2_ed82_u140j142_to_u142_rdata.h"
};
static constexpr size_t PatchLength_u140j142_to_u142_rdata =
    sizeof(PatchData_u140j142_to_u142_rdata);
static constexpr char PatchData_t1001_dat_to_tbl[] = {
#include "embed_sen2_t1001_dat_to_tbl.h"
};
static constexpr size_t PatchLength_t1001_dat_to_tbl = sizeof(PatchData_t1001_dat_to_tbl);
static constexpr char PatchData_t1001_j14_to_j142[] = {
    '\x42', '\x50', '\x53', '\x31', '\x51', '\x59', '\x89', '\x51', '\x59', '\x89', '\x80',
    '\x50', '\x13', '\x93', '\x81', '\xfe', '\x68', '\x54', '\x95', '\xd1', '\x34', '\x92',
    '\x1c', '\x08', '\x78', '\xd2', '\xa8', '\x99', '\xe0', '\x0c', '\x42'};
static constexpr size_t PatchLength_t1001_j14_to_j142 = sizeof(PatchData_t1001_j14_to_j142);
static constexpr char PatchData_t1001_u14_to_u142[] = {
    '\x42', '\x50', '\x53', '\x31', '\x11', '\x6a', '\x8a', '\x11', '\x6a', '\x8a', '\x80',
    '\x28', '\x76', '\x94', '\x81', '\xfe', '\x10', '\x34', '\x98', '\x4b', '\x18', '\x20',
    '\x67', '\x25', '\xf8', '\x4d', '\x9d', '\xc0', '\x33', '\x34', '\xcd'};
static constexpr size_t PatchLength_t1001_u14_to_u142 = sizeof(PatchData_t1001_u14_to_u142);
} // namespace

namespace {
struct SectionSplitExe {
    std::span<const char> header;
    std::span<const char> text;
    std::span<const char> rdata;
    std::span<const char> data;
    std::span<const char> rsrc;
};

std::optional<SectionSplitExe> ParseSectionSplitExe(std::span<const char> s) {
    HyoutaUtils::Stream::DuplicatableByteArrayStream b(s.data(), s.size());
    auto exe = SenTools::PeExe::ParsePeExe(b, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    if (!exe || exe->SectionHeaders.size() < 4) {
        return std::nullopt;
    }
    for (size_t i = 0; i < 4; ++i) {
        if (exe->SectionHeaders[i].PointerToRawData > s.size()) {
            return std::nullopt;
        }
        if (exe->SectionHeaders[i].SizeOfRawData
            > (s.size() - exe->SectionHeaders[i].PointerToRawData)) {
            return std::nullopt;
        }
    }

    SectionSplitExe split;
    split.header = std::span<const char>(s.data(), exe->SectionHeaders[0].PointerToRawData);
    split.text = std::span<const char>(s.data() + exe->SectionHeaders[0].PointerToRawData,
                                       exe->SectionHeaders[0].SizeOfRawData);
    split.rdata = std::span<const char>(s.data() + exe->SectionHeaders[1].PointerToRawData,
                                        exe->SectionHeaders[1].SizeOfRawData);
    split.data = std::span<const char>(s.data() + exe->SectionHeaders[2].PointerToRawData,
                                       exe->SectionHeaders[2].SizeOfRawData);
    split.rsrc = std::span<const char>(s.data() + exe->SectionHeaders[3].PointerToRawData,
                                       exe->SectionHeaders[3].SizeOfRawData);
    return split;
}
} // namespace

namespace SenTools {
enum class OnDiskFileId { Neither, First, Second };
struct OnDiskFile {
    OnDiskFileId Id = OnDiskFileId::Neither;
    std::vector<char> Data;
};

static OnDiskFile ReadKnownFileToBuffer(std::string_view path,
                                        const HyoutaUtils::Hash::SHA1& hash,
                                        size_t filesize,
                                        const HyoutaUtils::Hash::SHA1& althash,
                                        size_t altfilesize) {
    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return OnDiskFile();
    }
    auto length = f.GetLength();
    if (!length || !(*length == filesize || *length == altfilesize)) {
        return OnDiskFile();
    }
    std::vector<char> data;
    size_t realFileSize = static_cast<size_t>(*length);
    data.resize(realFileSize);
    if (f.Read(data.data(), realFileSize) != realFileSize) {
        return OnDiskFile();
    }
    auto realHash = HyoutaUtils::Hash::CalculateSHA1(data.data(), realFileSize);
    if (realFileSize == filesize && realHash == hash) {
        OnDiskFile result;
        result.Id = OnDiskFileId::First;
        result.Data = std::move(data);
        return result;
    }
    if (realFileSize == altfilesize && realHash == althash) {
        OnDiskFile result;
        result.Id = OnDiskFileId::Second;
        result.Data = std::move(data);
        return result;
    }
    return OnDiskFile();
}

static std::optional<std::vector<char>>
    ApplyPatch(std::span<const char> source, std::span<const char> patch, bool patchIsCompressed) {
    try {
        std::vector<char> result;
        if (patchIsCompressed) {
            auto decomp = SenLib::DecompressFromBuffer(patch.data(), patch.size());
            if (!decomp) {
                return std::nullopt;
            }
            HyoutaUtils::Stream::DuplicatableByteArrayStream s(source.data(), source.size());
            HyoutaUtils::Stream::DuplicatableByteArrayStream p(decomp->data(), decomp->size());
            HyoutaUtils::Bps::ApplyPatchToStream(s, p, result);
        } else {
            HyoutaUtils::Stream::DuplicatableByteArrayStream s(source.data(), source.size());
            HyoutaUtils::Stream::DuplicatableByteArrayStream p(patch.data(), patch.size());
            HyoutaUtils::Bps::ApplyPatchToStream(s, p, result);
        }
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

TryPatchCs2Version14Result TryPatchCs2Version14(std::string_view path,
                                                const std::function<bool()>& confirmationCallback) {
    if (path.empty()) {
        return TryPatchCs2Version14Result::IsNotV14;
    }

    std::string basepath(path);
#ifdef BUILD_FOR_WINDOWS
    if (!(basepath.ends_with('/') || basepath.ends_with('\\'))) {
#else
    if (!basepath.ends_with('/')) {
#endif
        basepath.push_back('/');
    }

    const std::string path_exe_jp = basepath + "bin/Win32/ed8_2_PC_JP.exe";
    const std::string path_exe_us = basepath + "bin/Win32/ed8_2_PC_US.exe";
    const std::string path_t1001_jp = basepath + "data/scripts/scena/dat/t1001.dat";
    const std::string path_t1001_us = basepath + "data/scripts/scena/dat_us/t1001.dat";
    const std::string path_asm_folder = basepath + "data/scripts/scena/asm";
    const std::string path_asm_file = basepath + "data/scripts/scena/asm/t1001.tbl";
    const std::string path_exe_jp_tmp = path_exe_jp + ".cs142";
    const std::string path_exe_us_tmp = path_exe_us + ".cs142";
    const std::string path_t1001_jp_tmp = path_t1001_jp + ".cs142";
    const std::string path_t1001_us_tmp = path_t1001_us + ".cs142";
    const std::string path_asm_file_tmp = path_asm_file + ".cs142";
    static constexpr auto jp142hash =
        HyoutaUtils::Hash::SHA1FromHexString("7d1db7e0bb91ab77a3fd1eba53b0ed25806186c1");
    static constexpr size_t jp142size = 6404096;
    static constexpr auto us142hash =
        HyoutaUtils::Hash::SHA1FromHexString("b08ece4ee38e6e3a99e58eb11cffb45e49704f86");
    static constexpr size_t us142size = 6421504;
    static constexpr auto t1001jp142hash =
        HyoutaUtils::Hash::SHA1FromHexString("24b90bc222efb431a05941973b3bcbd7e3599d81");
    static constexpr size_t t1001jp142size = 175441;
    static constexpr auto t1001us142hash =
        HyoutaUtils::Hash::SHA1FromHexString("fae1d23cd07aa0c990ca63607e64fcddd60a80da");
    static constexpr size_t t1001us142size = 193937;
    static constexpr auto t1001asm142hash =
        HyoutaUtils::Hash::SHA1FromHexString("568a1ae375a6077ef5c6fb8e277a333f1979505b");
    static constexpr size_t t1001asm142size = 15850;
    static constexpr auto us14hash =
        HyoutaUtils::Hash::SHA1FromHexString("e5f2e2682557af7a2f52b2299ba0980f218c5e66");
    static constexpr size_t us14size = 6313984;
    static constexpr auto jp14hash =
        HyoutaUtils::Hash::SHA1FromHexString("825e2643338963565f49e3c40aa0aec1d77229fa");
    static constexpr size_t jp14size = 6296064;
    static constexpr auto t1001us14hash =
        HyoutaUtils::Hash::SHA1FromHexString("ace845b437df94fbfe2d638a2ec162b492a657b3");
    static constexpr size_t t1001us14size = 193937;
    static constexpr auto t1001jp14hash =
        HyoutaUtils::Hash::SHA1FromHexString("3d75d79e3201f8f5ac61c206f8cc86db7c4651dd");
    static constexpr size_t t1001jp14size = 175441;

    // read all related files into memory
    auto on_disk_exe_us =
        ReadKnownFileToBuffer(path_exe_us, us14hash, us14size, us142hash, us142size);
    auto on_disk_exe_jp =
        ReadKnownFileToBuffer(path_exe_jp, jp14hash, jp14size, jp142hash, jp142size);
    auto on_disk_t1001_us = ReadKnownFileToBuffer(
        path_t1001_us, t1001us14hash, t1001us14size, t1001us142hash, t1001us142size);
    auto on_disk_t1001_jp = ReadKnownFileToBuffer(
        path_t1001_jp, t1001jp14hash, t1001jp14size, t1001jp142hash, t1001jp142size);
    auto on_disk_t1001_asm = ReadKnownFileToBuffer(
        path_asm_file, t1001asm142hash, t1001asm142size, t1001asm142hash, t1001asm142size);
    if (on_disk_exe_us.Id == OnDiskFileId::Second && on_disk_exe_jp.Id == OnDiskFileId::Second
        && on_disk_t1001_us.Id == OnDiskFileId::Second
        && on_disk_t1001_jp.Id == OnDiskFileId::Second
        && on_disk_t1001_asm.Id == OnDiskFileId::First) {
        // this is already CS2 1.4.2, nothing else to do here
        return TryPatchCs2Version14Result::IsNotV14;
    }
    auto on_disk_exe_us_tmp =
        ReadKnownFileToBuffer(path_exe_us_tmp, us14hash, us14size, us142hash, us142size);
    auto on_disk_exe_jp_tmp =
        ReadKnownFileToBuffer(path_exe_jp_tmp, jp14hash, jp14size, jp142hash, jp142size);
    auto on_disk_t1001_us_tmp = ReadKnownFileToBuffer(
        path_t1001_us_tmp, t1001us14hash, t1001us14size, t1001us142hash, t1001us142size);
    auto on_disk_t1001_jp_tmp = ReadKnownFileToBuffer(
        path_t1001_jp_tmp, t1001jp14hash, t1001jp14size, t1001jp142hash, t1001jp142size);
    auto on_disk_t1001_asm_tmp = ReadKnownFileToBuffer(
        path_asm_file_tmp, t1001asm142hash, t1001asm142size, t1001asm142hash, t1001asm142size);

    // can we assemble a full set for 1.4 and/or 1.4.2?
    std::optional<std::vector<char>> exe_us_14;
    std::optional<std::vector<char>> exe_jp_14;
    std::optional<std::vector<char>> t1001_us_14;
    std::optional<std::vector<char>> t1001_jp_14;
    std::optional<std::vector<char>> exe_us_142;
    std::optional<std::vector<char>> exe_jp_142;
    std::optional<std::vector<char>> t1001_us_142;
    std::optional<std::vector<char>> t1001_jp_142;
    std::optional<std::vector<char>> t1001_asm_142;
    if (on_disk_exe_us.Id == OnDiskFileId::First) {
        exe_us_14 = std::move(on_disk_exe_us.Data);
    }
    if (on_disk_exe_us.Id == OnDiskFileId::Second) {
        exe_us_142 = std::move(on_disk_exe_us.Data);
    }
    if (on_disk_exe_us_tmp.Id == OnDiskFileId::First) {
        exe_us_14 = std::move(on_disk_exe_us_tmp.Data);
    }
    if (on_disk_exe_us_tmp.Id == OnDiskFileId::Second) {
        exe_us_142 = std::move(on_disk_exe_us_tmp.Data);
    }
    if (on_disk_exe_jp.Id == OnDiskFileId::First) {
        exe_jp_14 = std::move(on_disk_exe_jp.Data);
    }
    if (on_disk_exe_jp.Id == OnDiskFileId::Second) {
        exe_jp_142 = std::move(on_disk_exe_jp.Data);
    }
    if (on_disk_exe_jp_tmp.Id == OnDiskFileId::First) {
        exe_jp_14 = std::move(on_disk_exe_jp_tmp.Data);
    }
    if (on_disk_exe_jp_tmp.Id == OnDiskFileId::Second) {
        exe_jp_142 = std::move(on_disk_exe_jp_tmp.Data);
    }
    if (on_disk_t1001_us.Id == OnDiskFileId::First) {
        t1001_us_14 = std::move(on_disk_t1001_us.Data);
    }
    if (on_disk_t1001_us.Id == OnDiskFileId::Second) {
        t1001_us_142 = std::move(on_disk_t1001_us.Data);
    }
    if (on_disk_t1001_us_tmp.Id == OnDiskFileId::First) {
        t1001_us_14 = std::move(on_disk_t1001_us_tmp.Data);
    }
    if (on_disk_t1001_us_tmp.Id == OnDiskFileId::Second) {
        t1001_us_142 = std::move(on_disk_t1001_us_tmp.Data);
    }
    if (on_disk_t1001_jp.Id == OnDiskFileId::First) {
        t1001_jp_14 = std::move(on_disk_t1001_jp.Data);
    }
    if (on_disk_t1001_jp.Id == OnDiskFileId::Second) {
        t1001_jp_142 = std::move(on_disk_t1001_jp.Data);
    }
    if (on_disk_t1001_jp_tmp.Id == OnDiskFileId::First) {
        t1001_jp_14 = std::move(on_disk_t1001_jp_tmp.Data);
    }
    if (on_disk_t1001_jp_tmp.Id == OnDiskFileId::Second) {
        t1001_jp_142 = std::move(on_disk_t1001_jp_tmp.Data);
    }
    if (on_disk_t1001_asm.Id == OnDiskFileId::First) {
        t1001_asm_142 = std::move(on_disk_t1001_asm.Data);
    }
    if (on_disk_t1001_asm_tmp.Id == OnDiskFileId::First) {
        t1001_asm_142 = std::move(on_disk_t1001_asm_tmp.Data);
    }

    // if we now have a full set of 1.4.2, it means our patching was interrupted the last time we
    // tried it. if this is the case, just proceed to the write step to recover this.
    if (!(exe_us_142 && exe_jp_142 && t1001_us_142 && t1001_jp_142 && t1001_asm_142)) {
        // do we have enough information to patch this?
        // for each file except the asm one we must have either version
        const bool have_exe_us = (exe_us_14 || exe_us_142);
        const bool have_exe_jp = (exe_jp_14 || exe_jp_142);
        const bool have_t1001_us = (t1001_us_14 || t1001_us_142);
        const bool have_t1001_jp = (t1001_jp_14 || t1001_jp_142);
        if (!(have_exe_us && have_exe_jp && have_t1001_us && have_t1001_jp)) {
            // not enough information. this is probably neither v1.4 nor 1.4.2
            return TryPatchCs2Version14Result::IsNotV14;
        }

        // ask user if they want to patch
        if (!confirmationCallback()) {
            // declined patch
            return TryPatchCs2Version14Result::UpdateDeclined;
        }

        // generate exe_jp_142 if we don't have it yet
        if (!exe_jp_142) {
            const auto jp140exe = ParseSectionSplitExe(*exe_jp_14);
            if (!jp140exe) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }

            auto jp142header = ApplyPatch(jp140exe->header,
                                          std::span<const char>(PatchData_j140_to_j142_header,
                                                                PatchLength_j140_to_j142_header),
                                          false);
            if (!jp142header) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto jp142text = ApplyPatch(
                jp140exe->text,
                std::span<const char>(PatchData_j140_to_j142_text, PatchLength_j140_to_j142_text),
                true);
            if (!jp142text) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto jp142rdata = ApplyPatch(
                jp140exe->rdata,
                std::span<const char>(PatchData_j140_to_j142_rdata, PatchLength_j140_to_j142_rdata),
                true);
            if (!jp142rdata) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto jp142data = ApplyPatch(
                jp140exe->data,
                std::span<const char>(PatchData_j140_to_j142_data, PatchLength_j140_to_j142_data),
                true);
            if (!jp142data) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto jp142rsrc = ApplyPatch(
                jp140exe->rsrc,
                std::span<const char>(PatchData_j140_to_j142_rsrc, PatchLength_j140_to_j142_rsrc),
                true);
            if (!jp142rsrc) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            std::vector<char> jp142;
            jp142.reserve(jp142header->size() + jp142text->size() + jp142rdata->size()
                          + jp142data->size() + jp142rsrc->size());
            jp142.insert(jp142.end(), jp142header->begin(), jp142header->end());
            jp142.insert(jp142.end(), jp142text->begin(), jp142text->end());
            jp142.insert(jp142.end(), jp142rdata->begin(), jp142rdata->end());
            jp142.insert(jp142.end(), jp142data->begin(), jp142data->end());
            jp142.insert(jp142.end(), jp142rsrc->begin(), jp142rsrc->end());
            if (HyoutaUtils::Hash::CalculateSHA1(jp142.data(), jp142.size()) != jp142hash) {
                // patch did not apply correctly, this is practically impossible
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            exe_jp_142 = std::move(jp142);
        }

        // generate exe_us_142 if we don't have it yet
        if (!exe_us_142) {
            const auto us140exe = ParseSectionSplitExe(*exe_us_14);
            if (!us140exe) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            const auto jp142exe = ParseSectionSplitExe(*exe_jp_142);
            if (!jp142exe) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }

            auto us142header = ApplyPatch(jp142exe->header,
                                          std::span<const char>(PatchData_j142_to_u142_header,
                                                                PatchLength_j142_to_u142_header),
                                          true);
            if (!us142header) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto us142text = ApplyPatch(
                jp142exe->text,
                std::span<const char>(PatchData_j142_to_u142_text, PatchLength_j142_to_u142_text),
                true);
            if (!us142text) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            std::vector<char> rdatatmp;
            rdatatmp.reserve(us140exe->rdata.size() + jp142exe->rdata.size());
            rdatatmp.insert(rdatatmp.end(), us140exe->rdata.begin(), us140exe->rdata.end());
            rdatatmp.insert(rdatatmp.end(), jp142exe->rdata.begin(), jp142exe->rdata.end());
            auto us142rdata = ApplyPatch(rdatatmp,
                                         std::span<const char>(PatchData_u140j142_to_u142_rdata,
                                                               PatchLength_u140j142_to_u142_rdata),
                                         true);
            if (!us142rdata) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto us142data = ApplyPatch(
                jp142exe->data,
                std::span<const char>(PatchData_j142_to_u142_data, PatchLength_j142_to_u142_data),
                true);
            if (!us142data) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            auto us142rsrc = ApplyPatch(
                jp142exe->rsrc,
                std::span<const char>(PatchData_j142_to_u142_rsrc, PatchLength_j142_to_u142_rsrc),
                false);
            if (!us142rsrc) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }

            std::vector<char> us142;
            us142.reserve(us142header->size() + us142text->size() + us142rdata->size()
                          + us142data->size() + us142rsrc->size());
            us142.insert(us142.end(), us142header->begin(), us142header->end());
            us142.insert(us142.end(), us142text->begin(), us142text->end());
            us142.insert(us142.end(), us142rdata->begin(), us142rdata->end());
            us142.insert(us142.end(), us142data->begin(), us142data->end());
            us142.insert(us142.end(), us142rsrc->begin(), us142rsrc->end());
            if (HyoutaUtils::Hash::CalculateSHA1(us142.data(), us142.size()) != us142hash) {
                // patch did not apply correctly, this is practically impossible
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            exe_us_142 = std::move(us142);
        }

        // generate t1001_jp_142 if we don't have it yet
        if (!t1001_jp_142) {
            t1001_jp_142 = ApplyPatch(
                *t1001_jp_14,
                std::span<const char>(PatchData_t1001_j14_to_j142, PatchLength_t1001_j14_to_j142),
                false);
            if (!t1001_jp_142) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            if (HyoutaUtils::Hash::CalculateSHA1(t1001_jp_142->data(), t1001_jp_142->size())
                != t1001jp142hash) {
                // patch did not apply correctly, this is practically impossible
                return TryPatchCs2Version14Result::PatchingFailed;
            }
        }

        // generate t1001_us_142 if we don't have it yet
        if (!t1001_us_142) {
            t1001_us_142 = ApplyPatch(
                *t1001_us_14,
                std::span<const char>(PatchData_t1001_u14_to_u142, PatchLength_t1001_u14_to_u142),
                false);
            if (!t1001_us_142) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            if (HyoutaUtils::Hash::CalculateSHA1(t1001_us_142->data(), t1001_us_142->size())
                != t1001us142hash) {
                // patch did not apply correctly, this is practically impossible
                return TryPatchCs2Version14Result::PatchingFailed;
            }
        }

        // generate t1001_asm_142 if we don't have it yet
        if (!t1001_asm_142) {
            t1001_asm_142 = ApplyPatch(
                *t1001_jp_142,
                std::span<const char>(PatchData_t1001_dat_to_tbl, PatchLength_t1001_dat_to_tbl),
                true);
            if (!t1001_asm_142) {
                return TryPatchCs2Version14Result::PatchingFailed;
            }
            if (HyoutaUtils::Hash::CalculateSHA1(t1001_asm_142->data(), t1001_asm_142->size())
                != t1001asm142hash) {
                // patch did not apply correctly, this is practically impossible
                return TryPatchCs2Version14Result::PatchingFailed;
            }
        }
    }

    // patching success, write out. we only need to write the files that aren't already on-disk.
    {
        HyoutaUtils::IO::CreateDirectory(
            std::string_view(path_asm_folder)); // may or may not already exist
        HyoutaUtils::IO::File newfile_exe_jp;
        if (on_disk_exe_jp.Id != OnDiskFileId::Second) {
            newfile_exe_jp.Open(std::string_view(path_exe_jp_tmp),
                                HyoutaUtils::IO::OpenMode::Write);
            if (!newfile_exe_jp.IsOpen()) {
                return TryPatchCs2Version14Result::WritingNewFilesFailed;
            }
        }
        HyoutaUtils::IO::File newfile_exe_us;
        if (on_disk_exe_us.Id != OnDiskFileId::Second) {
            newfile_exe_us.Open(std::string_view(path_exe_us_tmp),
                                HyoutaUtils::IO::OpenMode::Write);
            if (!newfile_exe_us.IsOpen()) {
                return TryPatchCs2Version14Result::WritingNewFilesFailed;
            }
        }
        HyoutaUtils::IO::File newfile_t1001_jp;
        if (on_disk_t1001_jp.Id != OnDiskFileId::Second) {
            newfile_t1001_jp.Open(std::string_view(path_t1001_jp_tmp),
                                  HyoutaUtils::IO::OpenMode::Write);
            if (!newfile_t1001_jp.IsOpen()) {
                return TryPatchCs2Version14Result::WritingNewFilesFailed;
            }
        }
        HyoutaUtils::IO::File newfile_t1001_us;
        if (on_disk_t1001_us.Id != OnDiskFileId::Second) {
            newfile_t1001_us.Open(std::string_view(path_t1001_us_tmp),
                                  HyoutaUtils::IO::OpenMode::Write);
            if (!newfile_t1001_us.IsOpen()) {
                return TryPatchCs2Version14Result::WritingNewFilesFailed;
            }
        }
        HyoutaUtils::IO::File newfile_asm_file;
        if (on_disk_t1001_asm.Id != OnDiskFileId::First) {
            newfile_asm_file.Open(std::string_view(path_asm_file_tmp),
                                  HyoutaUtils::IO::OpenMode::Write);
            if (!newfile_asm_file.IsOpen()) {
                return TryPatchCs2Version14Result::WritingNewFilesFailed;
            }
        }

        if (newfile_exe_jp.IsOpen()
            && newfile_exe_jp.Write(exe_jp_142->data(), exe_jp_142->size()) != exe_jp_142->size()) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_exe_us.IsOpen()
            && newfile_exe_us.Write(exe_us_142->data(), exe_us_142->size()) != exe_us_142->size()) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_t1001_jp.IsOpen()
            && newfile_t1001_jp.Write(t1001_jp_142->data(), t1001_jp_142->size())
                   != t1001_jp_142->size()) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_t1001_us.IsOpen()
            && newfile_t1001_us.Write(t1001_us_142->data(), t1001_us_142->size())
                   != t1001_us_142->size()) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_asm_file.IsOpen()
            && newfile_asm_file.Write(t1001_asm_142->data(), t1001_asm_142->size())
                   != t1001_asm_142->size()) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }

        if (newfile_exe_jp.IsOpen()
            && !(newfile_exe_jp.Flush() && newfile_exe_jp.Rename(std::string_view(path_exe_jp)))) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_exe_us.IsOpen()
            && !(newfile_exe_us.Flush() && newfile_exe_us.Rename(std::string_view(path_exe_us)))) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_t1001_jp.IsOpen()
            && !(newfile_t1001_jp.Flush()
                 && newfile_t1001_jp.Rename(std::string_view(path_t1001_jp)))) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_t1001_us.IsOpen()
            && !(newfile_t1001_us.Flush()
                 && newfile_t1001_us.Rename(std::string_view(path_t1001_us)))) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
        if (newfile_asm_file.IsOpen()
            && !(newfile_asm_file.Flush()
                 && newfile_asm_file.Rename(std::string_view(path_asm_file)))) {
            return TryPatchCs2Version14Result::WritingNewFilesFailed;
        }
    }

    // clean up any possible leftovers
    HyoutaUtils::IO::DeleteFile(std::string_view(path_exe_jp_tmp));
    HyoutaUtils::IO::DeleteFile(std::string_view(path_exe_us_tmp));
    HyoutaUtils::IO::DeleteFile(std::string_view(path_t1001_jp_tmp));
    HyoutaUtils::IO::DeleteFile(std::string_view(path_t1001_us_tmp));
    HyoutaUtils::IO::DeleteFile(std::string_view(path_asm_file_tmp));

    return TryPatchCs2Version14Result::UpdateSucceeded;
}
} // namespace SenTools
