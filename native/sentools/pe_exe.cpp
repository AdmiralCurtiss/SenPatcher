#include "pe_exe.h"

#include <memory>
#include <optional>
#include <vector>

#include "rom_mapper.h"
#include "util/endian.h"
#include "util/stream.h"

namespace SenTools::PeExe {
static bool ParseCoff(HyoutaUtils::Stream::DuplicatableStream& s,
                      HyoutaUtils::EndianUtils::Endianness e,
                      CoffData& coff) {
    coff.Machine = s.ReadUInt16(e);
    coff.NumberOfSections = s.ReadUInt16(e);
    coff.TimeDateStamp = s.ReadUInt32(e);
    coff.PointerToSymbolTable = s.ReadUInt32(e);
    coff.NumberOfSymbols = s.ReadUInt32(e);
    coff.SizeOfOptionalHeader = s.ReadUInt16(e);
    coff.Characteristics = s.ReadUInt16(e);
    return true;
}

static bool ParseImageDataDirectory(HyoutaUtils::Stream::DuplicatableStream& s,
                                    HyoutaUtils::EndianUtils::Endianness e,
                                    ImageDataDirectory& idd) {
    idd.VirtualAddress = s.ReadUInt32(e);
    idd.Size = s.ReadUInt32(e);
    return true;
}

static bool ParseOptionalHeader(HyoutaUtils::Stream::DuplicatableStream& s,
                                HyoutaUtils::EndianUtils::Endianness e,
                                OptionalHeaderData& oh) {
    oh.Magic = s.ReadUInt16(e);
    oh.MajorLinkerVersion = s.ReadUInt8();
    oh.MinorLinkerVersion = s.ReadUInt8();
    oh.SizeOfCode = s.ReadUInt32(e);
    oh.SizeOfInitializedData = s.ReadUInt32(e);
    oh.SizeOfUninitializedData = s.ReadUInt32(e);
    oh.AddressOfEntryPoint = s.ReadUInt32(e);
    oh.BaseOfCode = s.ReadUInt32(e);
    oh.BaseOfData = oh.Magic == OptionalHeaderData::PE32_MAGIC ? s.ReadUInt32(e) : 0;

    bool b64 = oh.Magic == OptionalHeaderData::PE32PLUS_MAGIC;
    oh.ImageBase = b64 ? s.ReadUInt64(e) : s.ReadUInt32(e);
    oh.SectionAlignment = s.ReadUInt32(e);
    oh.FileAlignment = s.ReadUInt32(e);
    oh.MajorOperatingSystemVersion = s.ReadUInt16(e);
    oh.MinorOperatingSystemVersion = s.ReadUInt16(e);
    oh.MajorImageVersion = s.ReadUInt16(e);
    oh.MinorImageVersion = s.ReadUInt16(e);
    oh.MajorSubsystemVersion = s.ReadUInt16(e);
    oh.MinorSubsystemVersion = s.ReadUInt16(e);
    oh.Win32VersionValue = s.ReadUInt32(e);
    oh.SizeOfImage = s.ReadUInt32(e);
    oh.SizeOfHeaders = s.ReadUInt32(e);
    oh.CheckSum = s.ReadUInt32(e);
    oh.Subsystem = s.ReadUInt16(e);
    oh.DllCharacteristics = s.ReadUInt16(e);
    oh.SizeOfStackReserve = b64 ? s.ReadUInt64(e) : s.ReadUInt32(e);
    oh.SizeOfStackCommit = b64 ? s.ReadUInt64(e) : s.ReadUInt32(e);
    oh.SizeOfHeapReserve = b64 ? s.ReadUInt64(e) : s.ReadUInt32(e);
    oh.SizeOfHeapCommit = b64 ? s.ReadUInt64(e) : s.ReadUInt32(e);
    oh.LoaderFlags = s.ReadUInt32(e);
    oh.NumberOfRvaAndSizes = s.ReadUInt32(e);

    for (uint32_t i = 0; i < oh.NumberOfRvaAndSizes; ++i) {
        ImageDataDirectory idd;
        if (!ParseImageDataDirectory(s, e, idd)) {
            return false;
        }
        if (i < oh.DataDirectories.size()) {
            oh.DataDirectories[i] = idd;
        }
    }

    return true;
}

bool ParseSectionHeader(HyoutaUtils::Stream::DuplicatableStream& s,
                        HyoutaUtils::EndianUtils::Endianness e,
                        SectionHeaderData& sh) {
    sh.Name = s.ReadArray<8>();
    sh.VirtualSize = s.ReadUInt32(e);
    sh.VirtualAddress = s.ReadUInt32(e);
    sh.SizeOfRawData = s.ReadUInt32(e);
    sh.PointerToRawData = s.ReadUInt32(e);
    sh.PointerToRelocations = s.ReadUInt32(e);
    sh.PointerToLinenumbers = s.ReadUInt32(e);
    sh.NumberOfRelocations = s.ReadUInt16(e);
    sh.NumberOfLinenumbers = s.ReadUInt16(e);
    sh.Characteristics = s.ReadUInt32(e);
    return true;
}

std::optional<PeExeData> ParsePeExe(HyoutaUtils::Stream::DuplicatableStream& s,
                                    HyoutaUtils::EndianUtils::Endianness e) {
    s.SetPosition(0x3c);
    uint32_t sigOffset = s.ReadUInt32(e);
    s.SetPosition(sigOffset);
    if (s.ReadUInt32(HyoutaUtils::EndianUtils::Endianness::LittleEndian) != 0x4550) {
        return std::nullopt;
    }
    PeExeData peExe;
    if (!ParseCoff(s, e, peExe.Coff)) {
        return std::nullopt;
    }
    uint64_t optionalHeaderStart = s.GetPosition();
    if (!ParseOptionalHeader(s, e, peExe.OptionalHeader)) {
        return std::nullopt;
    }
    s.SetPosition(optionalHeaderStart + peExe.Coff.SizeOfOptionalHeader);
    peExe.SectionHeaders.reserve(peExe.Coff.NumberOfSections);
    for (size_t i = 0; i < peExe.Coff.NumberOfSections; ++i) {
        auto& sh = peExe.SectionHeaders.emplace_back();
        if (!ParseSectionHeader(s, e, sh)) {
            return std::nullopt;
        }
    }

    return peExe;
}

SenTools::RomMapper CreateRomMapper(const PeExeData& peExe) {
    static constexpr auto mappedCharacteristics =
        (SectionHeaderData::IMAGE_SCN_CNT_CODE | SectionHeaderData::IMAGE_SCN_CNT_INITIALIZED_DATA);

    size_t count = 0;
    for (const SectionHeaderData& sh : peExe.SectionHeaders) {
        if ((sh.Characteristics & mappedCharacteristics) != 0) {
            ++count;
        }
    }
    std::unique_ptr<RomMapperRegion[]> regions = std::make_unique<RomMapperRegion[]>(count);
    size_t i = 0;
    for (const SectionHeaderData& sh : peExe.SectionHeaders) {
        if ((sh.Characteristics & mappedCharacteristics) != 0) {
            regions[i] =
                RomMapperRegion{.RomStart = sh.PointerToRawData,
                                .RamStart = peExe.OptionalHeader.ImageBase + sh.VirtualAddress,
                                .Length = sh.SizeOfRawData};
            ++i;
        }
    }

    return SenTools::RomMapper{.Regions = std::move(regions), .NumberOfRegions = count};
}

std::optional<std::vector<ImportTable>>
    GenerateDllImportList(HyoutaUtils::Stream::DuplicatableStream& s,
                          HyoutaUtils::EndianUtils::Endianness e,
                          const PeExeData& peExe) {
    const auto& oh = peExe.OptionalHeader;
    if (oh.ImageDataDirectoryIndexImportTable >= oh.DataDirectories.size()) {
        return std::nullopt;
    }
    const auto& importDataDir = oh.DataDirectories[oh.ImageDataDirectoryIndexImportTable];
    if (importDataDir.Size == 0) {
        return std::nullopt;
    }

    auto mapper = CreateRomMapper(peExe);
    uint64_t romAddress = mapper.MapRamToRom(oh.ImageBase + importDataDir.VirtualAddress).value();
    std::vector<ImportDirectoryEntry> importDirectoryEntries;
    s.SetPosition(romAddress);
    while (true) {
        uint32_t importLookupTableRVA = s.ReadUInt32(e);
        uint32_t timeDateStamp = s.ReadUInt32(e);
        uint32_t forwarderChain = s.ReadUInt32(e);
        uint32_t nameRVA = s.ReadUInt32(e);
        uint32_t importAddressTableRVA = s.ReadUInt32(e);
        bool isValid = importLookupTableRVA != 0 || timeDateStamp != 0 || forwarderChain != 0
                       || nameRVA != 0 || importAddressTableRVA != 0;
        if (isValid) {
            importDirectoryEntries.emplace_back(
                ImportDirectoryEntry{.ImportLookupTableRVA = importLookupTableRVA,
                                     .TimeDateStamp = timeDateStamp,
                                     .ForwarderChain = forwarderChain,
                                     .NameRVA = nameRVA,
                                     .ImportAddressTableRVA = importAddressTableRVA});
        } else {
            break;
        }
    }

    std::vector<ImportTable> importTables;
    for (const ImportDirectoryEntry& entry : importDirectoryEntries) {
        std::string dllName = s.ReadAsciiNulltermFromLocationAndReset(
            mapper.MapRamToRom(oh.ImageBase + entry.NameRVA).value());
        uint64_t lookupTableStart =
            mapper.MapRamToRom(oh.ImageBase + entry.ImportLookupTableRVA).value();
        s.SetPosition(lookupTableStart);

        std::vector<ImportTableEntry> imports;
        while (true) {
            const uint64_t ordinalMask = 0xffff;
            const uint64_t hintNameTableMask = 0x7fffffff;
            uint64_t flagMask = (oh.Magic == OptionalHeaderData::PE32PLUS_MAGIC)
                                    ? 0x8000000000000000ull
                                    : 0x80000000ull;
            uint64_t raw = (oh.Magic == OptionalHeaderData::PE32PLUS_MAGIC) ? s.ReadUInt64(e)
                                                                            : s.ReadUInt32(e);
            if (raw == 0) {
                break;
            }
            if ((raw & flagMask) != 0) {
                uint16_t ordinal = (uint16_t)(raw & ordinalMask);
                imports.emplace_back(
                    ImportTableEntry{.OrdinalOrHint = ordinal, .Name = std::nullopt});
            } else {
                uint32_t rva = (uint32_t)(raw & hintNameTableMask);
                uint64_t p = s.GetPosition();
                s.SetPosition(mapper.MapRamToRom(oh.ImageBase + rva).value());
                uint16_t hint = s.ReadUInt16(e);
                std::string name = s.ReadAsciiNullterm();
                s.SetPosition(p);
                imports.emplace_back(
                    ImportTableEntry{.OrdinalOrHint = hint, .Name = std::move(name)});
            }
        }

        uint64_t lookupTableEnd = s.GetPosition();
        uint64_t lookupTableLength = lookupTableEnd - lookupTableStart;

        // verify lookup table == address table
        s.SetPosition(mapper.MapRamToRom(oh.ImageBase + entry.ImportLookupTableRVA).value());
        auto lookup = s.ReadVector(lookupTableLength);
        s.SetPosition(mapper.MapRamToRom(oh.ImageBase + entry.ImportAddressTableRVA).value());
        auto address = s.ReadVector(lookupTableLength);

        if (lookup != address) {
            return std::nullopt;
        }

        importTables.emplace_back(
            ImportTable{.DllName = std::move(dllName), .ImportedObjects = std::move(imports)});
    }

    return importTables;
}

std::optional<ExportTable> GenerateDllExportList(HyoutaUtils::Stream::DuplicatableStream& s,
                                                 HyoutaUtils::EndianUtils::Endianness e,
                                                 const PeExeData& peExe) {
    const auto& oh = peExe.OptionalHeader;
    if (oh.ImageDataDirectoryIndexExportTable >= oh.DataDirectories.size()) {
        return std::nullopt;
    }
    const auto& exportDataDir = oh.DataDirectories[oh.ImageDataDirectoryIndexExportTable];
    if (exportDataDir.Size == 0) {
        return std::nullopt;
    }

    uint64_t exportSectionStart = oh.ImageBase + exportDataDir.VirtualAddress;
    uint32_t exportSectionSize = exportDataDir.Size;

    ExportTable exportTable;
    auto mapper = CreateRomMapper(peExe);
    uint64_t romAddress = mapper.MapRamToRom(exportSectionStart).value();
    ExportDirectory exportDirectory;
    s.SetPosition(romAddress);
    exportDirectory.Characteristics = s.ReadUInt32(e);
    exportDirectory.TimeDateStamp = s.ReadUInt32(e);
    exportDirectory.MajorVersion = s.ReadUInt16(e);
    exportDirectory.MinorVersion = s.ReadUInt16(e);
    exportDirectory.NameRVA = s.ReadUInt32(e);
    exportDirectory.OrdinalBase = s.ReadUInt32(e);
    exportDirectory.NumberOfAddressTableEntries = s.ReadUInt32(e);
    exportDirectory.NumberOfNamePointers = s.ReadUInt32(e);
    exportDirectory.ExportAddressTableRVA = s.ReadUInt32(e);
    exportDirectory.NamePointerRVA = s.ReadUInt32(e);
    exportDirectory.OrdinalTableRVA = s.ReadUInt32(e);

    exportTable.Name = s.ReadAsciiNulltermFromLocationAndReset(
        mapper.MapRamToRom(oh.ImageBase + exportDirectory.NameRVA).value());
    exportTable.OridinalBase = exportDirectory.OrdinalBase;

    uint64_t romAddressFunctions =
        mapper.MapRamToRom(oh.ImageBase + exportDirectory.ExportAddressTableRVA).value();
    uint64_t romAddressNames =
        mapper.MapRamToRom(oh.ImageBase + exportDirectory.NamePointerRVA).value();
    uint64_t romAddressNameOrdinals =
        mapper.MapRamToRom(oh.ImageBase + exportDirectory.OrdinalTableRVA).value();

    std::vector<AddressTableEntry> addressTable;
    std::vector<NameLookupTableEntry> nameLookupTable;

    s.SetPosition(romAddressFunctions);
    for (size_t i = 0; i < exportDirectory.NumberOfAddressTableEntries; ++i) {
        uint64_t address = oh.ImageBase + s.ReadUInt32(e);
        if (address >= exportSectionStart && address < (exportSectionStart + exportSectionSize)) {
            std::string forwarder =
                s.ReadAsciiNulltermFromLocationAndReset(mapper.MapRamToRom(address).value());
            addressTable.emplace_back(AddressTableEntry{.Data = std::move(forwarder)});
        } else {
            addressTable.emplace_back(AddressTableEntry{.Data = address});
        }
    }
    for (size_t i = 0; i < exportDirectory.NumberOfNamePointers; ++i) {
        s.SetPosition(romAddressNames + i * 4);
        uint32_t nameLocation = s.ReadUInt32(e);
        s.SetPosition(romAddressNameOrdinals + i * 2);
        uint16_t ordinal = s.ReadUInt16(e);
        std::string name = s.ReadAsciiNulltermFromLocationAndReset(
            mapper.MapRamToRom(oh.ImageBase + nameLocation).value());
        nameLookupTable.emplace_back(
            NameLookupTableEntry{.Name = std::move(name), .Index = ordinal});
    }

    exportTable.AddressTable = std::move(addressTable);
    exportTable.NameLookupTable = std::move(nameLookupTable);

    return exportTable;
}
} // namespace SenTools::PeExe
