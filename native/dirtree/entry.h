#pragma once

#include <cassert>
#include <cstdint>

namespace HyoutaUtils::DirTree {
struct Entry {
    // This design is massively overcomplicated, to be honest, but it results in a rather compact
    // storage...

    // This is:
    // 8 bits of filename length (in bytes)
    // 24 bits of filename index in string table
    uint32_t Filename;

    // This is:
    // 20 bits of version bitfield, ie. which game version this file is contained in
    // 11 bits of DLC index, ie. if this is a DLC file and which DLC package it is
    // 1 bit of whether this is a file or a folder
    uint32_t Metadata;

    union {
        struct {
            // This is:
            // 20 bits of index of hash in hash table
            // 44 bits of filesize
            uint64_t HashAndSize;
        } File;

        struct {
            // Index of first child in Entry table
            uint32_t FirstEntry;

            // Number of children in this directory (not recursive)
            uint32_t NumberOfEntries;
        } Directory;
    } Data;

    constexpr Entry(uint32_t filename, uint32_t metadata, uint64_t fileHashAndSize)
      : Filename(filename), Metadata(metadata), Data{.File{.HashAndSize = fileHashAndSize}} {}

    constexpr Entry(uint32_t filename,
                    uint32_t metadata,
                    uint32_t directoryFirstEntry,
                    uint32_t directoryNumberOfEntries)
      : Filename(filename)
      , Metadata(metadata)
      , Data{.Directory{.FirstEntry = directoryFirstEntry,
                        .NumberOfEntries = directoryNumberOfEntries}} {}

    constexpr static Entry MakeFile(uint32_t filenameOffset,
                                    uint32_t filenameLength,
                                    uint32_t gameVersionBits,
                                    uint32_t dlcIndex,
                                    uint32_t hashIndex,
                                    uint64_t filesize) {
        assert(filenameOffset < (1 << 24));
        assert(filenameLength < (1 << 8));
        assert(gameVersionBits < (1 << 20));
        assert(dlcIndex < (1 << 11));
        assert(filesize < (1ull << 44));
        assert(hashIndex < (1 << 20));

        uint32_t filename = (filenameOffset << 8) | filenameLength;
        uint32_t metadata = (dlcIndex << 20) | gameVersionBits;
        uint64_t hashAndSize = (filesize << 20) | hashIndex;
        return Entry(filename, metadata, hashAndSize);
    }

    constexpr static Entry MakeDirectory(uint32_t filenameOffset,
                                         uint32_t filenameLength,
                                         uint32_t gameVersionBits,
                                         uint32_t dlcIndex,
                                         uint32_t directoryFirstEntry,
                                         uint32_t directoryNumberOfEntries) {
        assert(filenameOffset < (1 << 24));
        assert(filenameLength < (1 << 8));
        assert(gameVersionBits < (1 << 20));
        assert(dlcIndex < (1 << 11));

        uint32_t filename = (filenameOffset << 8) | filenameLength;
        uint32_t metadata = (dlcIndex << 20) | gameVersionBits | 0x8000'0000u;
        return Entry(filename, metadata, directoryFirstEntry, directoryNumberOfEntries);
    }
};
} // namespace HyoutaUtils::DirTree
