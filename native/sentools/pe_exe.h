#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "rom_mapper.h"
#include "util/endian.h"
#include "util/stream.h"

namespace SenTools::PeExe {
struct CoffData {
    uint16_t Machine = 0;
    uint16_t NumberOfSections = 0;
    uint32_t TimeDateStamp = 0;
    uint32_t PointerToSymbolTable = 0;
    uint32_t NumberOfSymbols = 0;
    uint16_t SizeOfOptionalHeader = 0;
    uint16_t Characteristics = 0;
};

struct ImageDataDirectory {
    uint32_t VirtualAddress = 0;
    uint32_t Size = 0;
};

struct OptionalHeaderData {
    uint16_t Magic = 0;
    uint8_t MajorLinkerVersion = 0;
    uint8_t MinorLinkerVersion = 0;
    uint32_t SizeOfCode = 0;
    uint32_t SizeOfInitializedData = 0;
    uint32_t SizeOfUninitializedData = 0;
    uint32_t AddressOfEntryPoint = 0;
    uint32_t BaseOfCode = 0;
    uint32_t BaseOfData = 0;

    uint64_t ImageBase = 0;
    uint32_t SectionAlignment = 0;
    uint32_t FileAlignment = 0;
    uint16_t MajorOperatingSystemVersion = 0;
    uint16_t MinorOperatingSystemVersion = 0;
    uint16_t MajorImageVersion = 0;
    uint16_t MinorImageVersion = 0;
    uint16_t MajorSubsystemVersion = 0;
    uint16_t MinorSubsystemVersion = 0;
    uint32_t Win32VersionValue = 0;
    uint32_t SizeOfImage = 0;
    uint32_t SizeOfHeaders = 0;
    uint32_t CheckSum = 0;
    uint16_t Subsystem = 0;
    uint16_t DllCharacteristics = 0;
    uint64_t SizeOfStackReserve = 0;
    uint64_t SizeOfStackCommit = 0;
    uint64_t SizeOfHeapReserve = 0;
    uint64_t SizeOfHeapCommit = 0;
    uint32_t LoaderFlags = 0;
    uint32_t NumberOfRvaAndSizes = 0;

    std::array<ImageDataDirectory, 16> DataDirectories;

    static constexpr int ImageDataDirectoryIndexExportTable = 0;
    static constexpr int ImageDataDirectoryIndexImportTable = 1;
    static constexpr int ImageDataDirectoryIndexResourceTable = 2;
    static constexpr int ImageDataDirectoryIndexExceptionTable = 3;
    static constexpr int ImageDataDirectoryIndexCertificateTable = 4;
    static constexpr int ImageDataDirectoryIndexBaseRelocationTable = 5;
    static constexpr int ImageDataDirectoryIndexDebug = 6;
    static constexpr int ImageDataDirectoryIndexArchitecture = 7;
    static constexpr int ImageDataDirectoryIndexGlobalPtr = 8;
    static constexpr int ImageDataDirectoryIndexTLSTable = 9;
    static constexpr int ImageDataDirectoryIndexLoadConfigTable = 10;
    static constexpr int ImageDataDirectoryIndexBoundImport = 11;
    static constexpr int ImageDataDirectoryIndexIAT = 12;
    static constexpr int ImageDataDirectoryIndexDelayImportDescriptor = 13;
    static constexpr int ImageDataDirectoryIndexCLRRuntimeHeader = 14;
    static constexpr int ImageDataDirectoryIndexReserved = 15;

    static constexpr uint16_t PE32_MAGIC = 0x10b;
    static constexpr uint16_t PE32PLUS_MAGIC = 0x20b;
};

struct SectionHeaderData {
    std::array<char, 8> Name{};
    uint32_t VirtualSize = 0;
    uint32_t VirtualAddress = 0;
    uint32_t SizeOfRawData = 0;
    uint32_t PointerToRawData = 0;
    uint32_t PointerToRelocations = 0;
    uint32_t PointerToLinenumbers = 0;
    uint16_t NumberOfRelocations = 0;
    uint16_t NumberOfLinenumbers = 0;
    uint32_t Characteristics = 0;

    static constexpr uint32_t IMAGE_SCN_CNT_CODE = 0x00000020u;
    static constexpr uint32_t IMAGE_SCN_CNT_INITIALIZED_DATA = 0x00000040u;
    static constexpr uint32_t IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080u;
    static constexpr uint32_t IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_DISCARDABLE = 0x02000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_NOT_CACHED = 0x04000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_NOT_PAGED = 0x08000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_SHARED = 0x10000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_EXECUTE = 0x20000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_READ = 0x40000000u;
    static constexpr uint32_t IMAGE_SCN_MEM_WRITE = 0x80000000u;
};

struct ImportDirectoryEntry {
    uint32_t ImportLookupTableRVA = 0;
    uint32_t TimeDateStamp = 0;
    uint32_t ForwarderChain = 0;
    uint32_t NameRVA = 0;
    uint32_t ImportAddressTableRVA = 0;
};

struct ExportDirectory {
    uint32_t Characteristics = 0;
    uint32_t TimeDateStamp = 0;
    uint16_t MajorVersion = 0;
    uint16_t MinorVersion = 0;
    uint32_t NameRVA = 0;
    uint32_t OrdinalBase = 0;
    uint32_t NumberOfAddressTableEntries = 0;
    uint32_t NumberOfNamePointers = 0;
    uint32_t ExportAddressTableRVA = 0;
    uint32_t NamePointerRVA = 0;
    uint32_t OrdinalTableRVA = 0;
};

struct ImportTableEntry {
    uint16_t OrdinalOrHint = 0;
    std::optional<std::string> Name;
};

struct ImportTable {
    std::string DllName;
    std::vector<ImportTableEntry> ImportedObjects;
};

struct AddressTableEntry {
    // uint64_t -> Address of data in current DLL
    // string -> Forwarder name to other DLL
    std::variant<uint64_t, std::string> Data = uint64_t(0);
};

struct NameLookupTableEntry {
    std::string Name;
    uint16_t Index = 0;
};

struct ExportTable {
    std::string Name;
    uint32_t OridinalBase = 0; // Ordinal == NameLookupTable[n].Index + OrdinalBase
    std::vector<AddressTableEntry> AddressTable;
    std::vector<NameLookupTableEntry> NameLookupTable;
};

struct PeExeData {
    CoffData Coff;
    OptionalHeaderData OptionalHeader;
    std::vector<SectionHeaderData> SectionHeaders;
};

std::optional<PeExeData> ParsePeExe(HyoutaUtils::Stream::DuplicatableStream& s,
                                    HyoutaUtils::EndianUtils::Endianness e);
std::optional<std::vector<ImportTable>>
    GenerateDllImportList(HyoutaUtils::Stream::DuplicatableStream& s,
                          HyoutaUtils::EndianUtils::Endianness e,
                          const PeExeData& peExe);
std::optional<ExportTable> GenerateDllExportList(HyoutaUtils::Stream::DuplicatableStream& s,
                                                 HyoutaUtils::EndianUtils::Endianness e,
                                                 const PeExeData& peExe);
SenTools::RomMapper CreateRomMapper(const PeExeData& peExe);
} // namespace SenTools::PeExe
