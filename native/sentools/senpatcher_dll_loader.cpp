#include "senpatcher_dll_loader.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "pe_exe.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/stream.h"
#include "util/text.h"

namespace SenTools {
SenPatcherDllIdentificationResult IdentifySenPatcherDll(std::string_view path) {
    if (HyoutaUtils::IO::FileExists(path) != HyoutaUtils::IO::ExistsResult::DoesExist) {
        SenPatcherDllIdentificationResult result;
        result.Type = SenPatcherDllIdentificationType::FileDoesNotExist;
        return result;
    }
    HyoutaUtils::IO::File f(path, HyoutaUtils::IO::OpenMode::Read);
    return IdentifySenPatcherDll(f);
}

SenPatcherDllIdentificationResult IdentifySenPatcherDll(HyoutaUtils::IO::File& f) {
    SenPatcherDllIdentificationResult result;
    if (!f.IsOpen()) {
        result.Type = SenPatcherDllIdentificationType::FileInaccessible;
        return result;
    }
    if (!f.SetPosition(0)) {
        result.Type = SenPatcherDllIdentificationType::FileInaccessible;
        return result;
    }
    const auto filesize = f.GetLength();
    if (!filesize) {
        result.Type = SenPatcherDllIdentificationType::FileDoesNotExist;
        return result;
    }
    if (*filesize > (50u * 1024u * 1024u)) {
        // too big to be a senpatcher DLL
        result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
        return result;
    }
    const size_t szlen = static_cast<size_t>(*filesize);

    auto data = std::make_unique_for_overwrite<char[]>(szlen);
    if (!data) {
        result.Type = SenPatcherDllIdentificationType::UnspecifiedError;
        return result;
    }
    if (f.Read(data.get(), szlen) != szlen) {
        result.Type = SenPatcherDllIdentificationType::FileInaccessible;
        return result;
    }

    HyoutaUtils::Stream::DuplicatableByteArrayStream stream(data.get(), szlen);
    auto peExe =
        SenTools::PeExe::ParsePeExe(stream, HyoutaUtils::EndianUtils::Endianness::LittleEndian);
    if (!peExe || ((peExe->Coff.Characteristics & 0x2002) != 0x2002)) {
        // not a valid DLL
        result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
        return result;
    }

    auto dll_export = SenTools::PeExe::GenerateDllExportList(
        stream, HyoutaUtils::EndianUtils::Endianness::LittleEndian, *peExe);
    if (!dll_export) {
        // no exports, can't be a SenPatcher DLL
        result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
        return result;
    }

    bool isDINPUT8 = false;
    bool isDSOUND = false;
    int sengame = -1; // not detected
    auto mapper = SenTools::PeExe::CreateRomMapper(*peExe);
    const auto get_string_value =
        [&](const PeExe::NameLookupTableEntry& nameLookup) -> std::optional<std::string_view> {
        if (nameLookup.Index >= dll_export->AddressTable.size()) {
            return std::nullopt;
        }
        const auto& entry = dll_export->AddressTable[nameLookup.Index];
        if (!std::holds_alternative<uint64_t>(entry.Data)) {
            return std::nullopt;
        }
        auto mappedAddress = mapper.MapRamToRom(std::get<uint64_t>(entry.Data));
        if (!mappedAddress || *mappedAddress >= szlen) {
            return std::nullopt;
        }
        return HyoutaUtils::TextUtils::StripToNull(
            std::string_view(data.get() + *mappedAddress, data.get() + szlen));
    };
    for (const auto& nameLookup : dll_export->NameLookupTable) {
        if (nameLookup.Name == "DirectInput8Create") {
            isDINPUT8 = true;
        } else if (nameLookup.Name == "DirectSoundCreate8") {
            isDSOUND = true;
        } else if (nameLookup.Name == "SenPatcherHook") {
            auto value = get_string_value(nameLookup);
            if (!value) {
                // not a valid string exported, not a senpatcher dll
                result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
                return result;
            }
            if (*value == "Sen1") {
                sengame = 1;
            } else if (*value == "Sen2") {
                sengame = 2;
            } else if (*value == "Sen3") {
                sengame = 3;
            } else if (*value == "Sen4") {
                sengame = 4;
            } else if (*value == "Sen5") {
                sengame = 5; // Reverie
            } else if (*value == "TX") {
                sengame = 6; // Tokyo Xanadu
            } else {
                sengame = 0; // Unknown game
            }
        } else if (nameLookup.Name == "SenPatcherVersion") {
            auto value = get_string_value(nameLookup);
            if (!value) {
                // not a valid string exported, not a senpatcher dll
                result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
                return result;
            }
            result.Version = std::string(*value);
        } else if (nameLookup.Name.starts_with("SenPatcherFix_")) {
            auto value = get_string_value(nameLookup);
            if (!value) {
                // not a valid string exported, not a senpatcher dll
                result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll;
                return result;
            }
            if (!result.FileFixInfo.has_value()) {
                result.FileFixInfo = std::string(*value);
            } else {
                result.FileFixInfo->push_back('\n');
                result.FileFixInfo->append(*value);
            }
        }
    }
    if (sengame == -1 && (isDSOUND || isDINPUT8)) {
        // Not recognized, but is a DSOUND or DINPUT8 dll. This could be an older SenPatcher version
        // before we introduced the exported strings, see if we can find something recognizable in
        // the .rdata section.
        for (const auto& sectionHeader : peExe->SectionHeaders) {
            if (HyoutaUtils::TextUtils::StripToNull(
                    std::string_view(sectionHeader.Name.data(), sectionHeader.Name.size()))
                    == ".rdata"
                && sectionHeader.PointerToRawData < szlen
                && sectionHeader.SizeOfRawData <= szlen - sectionHeader.PointerToRawData) {
                std::string_view rdata(data.get() + sectionHeader.PointerToRawData,
                                       data.get() + sectionHeader.PointerToRawData
                                           + sectionHeader.SizeOfRawData);
                std::string_view marker = " hook from SenPatcher, version ";
                auto stringloc = rdata.find(marker);
                if (stringloc != std::string_view::npos) {
                    // found the log string, extract our version from that
                    std::string_view v = HyoutaUtils::TextUtils::StripToNull(
                        rdata.substr(stringloc + marker.size()));
                    while (!v.empty() && (v.back() == '\n' || v.back() == '.')) {
                        v = v.substr(0, v.size() - 1);
                    }
                    if (!v.empty()) {
                        // okay, we got the version, now also take the word before the marker string
                        // to identify the hook itself
                        std::string_view h = rdata.substr(0, stringloc);
                        auto spaceloc = h.find_last_of(' ');
                        if (spaceloc != std::string::npos) {
                            h = h.substr(spaceloc + 1);
                            if (h == "CS1") {
                                sengame = 1;
                            } else if (h == "CS2") {
                                sengame = 2;
                            } else if (h == "CS3") {
                                sengame = 3;
                            } else if (h == "CS4") {
                                sengame = 4;
                            } else if (h == "Reverie") {
                                sengame = 5;
                            } else if (h == "Xanadu") {
                                sengame = 6;
                            } else {
                                sengame = 0; // Unknown game
                            }
                            result.Version = v;
                        }
                    }
                }
            }
        }
    }
    switch (sengame) {
        case 0: result.Type = SenPatcherDllIdentificationType::UnknownHook; break;
        case 1: result.Type = SenPatcherDllIdentificationType::CS1Hook; break;
        case 2: result.Type = SenPatcherDllIdentificationType::CS2Hook; break;
        case 3: result.Type = SenPatcherDllIdentificationType::CS3Hook; break;
        case 4: result.Type = SenPatcherDllIdentificationType::CS4Hook; break;
        case 5: result.Type = SenPatcherDllIdentificationType::ReverieHook; break;
        case 6: result.Type = SenPatcherDllIdentificationType::TXHook; break;
        default: result.Type = SenPatcherDllIdentificationType::NotASenPatcherDll; break;
    }
    return result;
}
} // namespace SenTools
