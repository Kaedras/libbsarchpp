#pragma once

#include "constants.h"
#include "enums.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <openssl/evp.h>
#include <string_view>
#include <variant>
#include <vector>

namespace libbsarchpp {

// forward declarations
class Bsa;
struct TexChunkRec;
struct FileTES3;
struct FolderTES4;
struct FileTES4;
struct FileFO4;

struct fileDeleter
{
    void operator()(FILE *f) { fclose(f); }
};

using FileRecord_t = std::variant<std::nullptr_t, FileTES3 *, FileTES4 *, FileFO4 *, TexChunkRec *>;
using FilePtr_t = std::variant<std::nullptr_t, FileTES3 *, FileTES4 *, FileFO4 *>;
using filePtr = std::unique_ptr<FILE, fileDeleter>;

using Magic4 = std::array<char, 4>; // fourCC
using Buffer = std::vector<uint8_t>;
using PackedDataHash = std::array<unsigned char, EVP_MAX_MD_SIZE>;

namespace typeSizes {
inline constexpr uint32_t DDS_PIXELFORMAT = 32;
inline constexpr uint32_t DDSHeader = 128;
inline constexpr uint32_t DDSHeaderDX10 = 20;
inline constexpr uint32_t HeaderTES3 = 8;
inline constexpr uint32_t HeaderTES4 = 28;
inline constexpr uint32_t HeaderFO4 = 16;
inline constexpr uint32_t HeaderSF = 8;
inline constexpr uint32_t HeaderSFdds = 12;
inline constexpr uint32_t TexChunkRec = 20;
} // namespace typeSizes

// we want to be able to directly read and write structs, so we have to disable padding
#ifdef __GNUC__
#define PACKED(structure) structure __attribute__((__packed__))
#else
#define PACKED(structure) __pragma(pack(push, 1)) structure __pragma(pack(pop))
#endif

//NOTE: string types use Windows-1252 encoding.

PACKED(struct DDS_PIXELFORMAT {
    uint32_t size = 0;
    uint32_t flags = 0;
    uint32_t fourCC = 0;
    uint32_t RGBBitCount = 0;
    uint32_t RBitMask = 0;
    uint32_t GBitMask = 0;
    uint32_t BBitMask = 0;
    uint32_t ABitMask = 0;
});
static_assert(sizeof(DDS_PIXELFORMAT) == typeSizes::DDS_PIXELFORMAT);

#define RESERVED1_ARRAY std::array<uint32_t, 11> // required to use multiple template arguments inside macro
PACKED(struct DDSHeader {
    uint32_t magic = 0;
    uint32_t size = 0;
    uint32_t flags = 0;
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitchOrLinearSize = 0;
    uint32_t depth = 0;
    uint32_t mipMapCount = 0;
    RESERVED1_ARRAY reserved1 = {};
    DDS_PIXELFORMAT ddspf;

    uint32_t caps = 0;
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;
});
static_assert(sizeof(DDSHeader) == typeSizes::DDSHeader);
#undef RESERVED1_ARRAY

PACKED(struct DDSHeaderDX10 {
    int32_t dxgiFormat = 0;
    uint32_t resourceDimension = 0;
    uint32_t miscFlags = 0;
    uint32_t arraySize = 0;
    uint32_t miscFlags2 = 0;
});
static_assert(sizeof(DDSHeaderDX10) == typeSizes::DDSHeaderDX10);

struct DDSInfo
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t mipMaps = 0;
};

PACKED(struct HeaderTES3 {
    uint32_t hashOffset = 0;
    uint32_t fileCount = 0;
});
static_assert(sizeof(HeaderTES3) == typeSizes::HeaderTES3);

struct FileTES3
{
    uint64_t hash = 0;
    uint32_t size = 0;
    uint32_t offset = 0;
    std::filesystem::path name; // Windows-1252 string
};

PACKED(struct HeaderTES4 {
    uint32_t foldersOffset = 0;
    uint32_t flags = 0;
    uint32_t folderCount = 0;
    uint32_t fileCount = 0;
    uint32_t folderNamesLength = 0;
    uint32_t fileNamesLength = 0;
    uint32_t fileFlags = 0;
});
static_assert(sizeof(HeaderTES4) == typeSizes::HeaderTES4);

struct FileTES4
{
    uint64_t hash = 0;
    uint32_t size = 0;
    uint32_t offset = 0;
    std::filesystem::path name; // Windows-1252 string
    PackingCompression_t packingCompression = PackingCompression_t::global;
    [[nodiscard]] bool compress(const Bsa *bsa) const noexcept; // compress when packing into a new archive
};

struct FolderTES4
{
    uint64_t hash = 0;
    uint32_t fileCount = 0;
    uint32_t unk32 = 0;
    uint64_t offset = 0;
    std::filesystem::path name; // Windows-1252 string
    std::vector<FileTES4> files;
};

PACKED(struct HeaderFO4 {
    uint32_t magic = 0;
    uint32_t fileCount = 0;
    int64_t fileTableOffset = 0;
});
static_assert(sizeof(HeaderFO4) == typeSizes::HeaderFO4);

PACKED(struct HeaderSF {
    HeaderFO4 fo4Header = {};
    uint32_t unknown1 = 0;
    uint32_t unknown2 = 0;
});
static_assert(sizeof(HeaderSF) == typeSizes::HeaderFO4 + typeSizes::HeaderSF);

PACKED(struct HeaderSFdds {
    HeaderFO4 fo4Header = {};
    uint32_t unknown1 = 0;
    uint32_t unknown2 = 0;
    uint32_t compressionMethod = 0;
});
static_assert(sizeof(HeaderSFdds) == typeSizes::HeaderFO4 + typeSizes::HeaderSFdds);

PACKED(struct TexChunkRec {
    int64_t offset = 0;
    uint32_t packedSize = 0;
    uint32_t size = 0;
    uint16_t startMip = 0;
    uint16_t endMip = 0;
});
static_assert(sizeof(TexChunkRec) == typeSizes::TexChunkRec);

struct FileFO4
{
    uint32_t nameHash = 0;
    std::array<char, 4> ext{0, 0, 0, 0};
    uint32_t dirHash = 0;
    // GNRL archive format
    uint32_t unknown = 0;
    int64_t offset = 0;
    uint32_t packedSize = 0;
    uint32_t size = 0;
    // DX10 archive format
    uint8_t unknownTex = 0;
    // uint16_t chunkHeaderSize; this value is a constant (24)
    uint16_t height = 0;
    uint16_t width = 0;
    uint8_t numMips = 0;
    uint8_t dxgiFormat = 0;
    uint16_t cubeMaps = 0;
    std::vector<TexChunkRec> texChunks;

    std::filesystem::path name; // Windows-1252 string
    PackingCompression_t packingCompression = PackingCompression_t::global;

    [[nodiscard]] std::string_view dxgiFormatName() const noexcept;
    [[nodiscard]] bool compress(const Bsa *bsa) const noexcept; // compress when packing into a new archive
};

struct PackedDataInfo
{
    uint32_t size = 0;
    PackedDataHash hash{};
    FileRecord_t fileRecord = nullptr;
};

/**
 * Advanced settings for bsa creation
 */
struct BsaCreationSettings
{
    /**
     * Enable multithreading. Increases performance, but produces indeterministic results.
     */
    bool multithreaded = false;
    /**
     * Enable compression. Reduces file size and performance.
     */
    bool compressed = false;
    int compressionLevel = 0;
    /**
     * Identical files will only be written once. May reduce filesize and may either reduce or increase performance.
     */
    bool shareData = false;
    bool ignoreExtensionlessFiles = true; /**< don't add files without extensions to archive */
    std::vector<std::string> extensionBlacklist{".exe", ".bsa", ".ba2", ".db"};
};
} // namespace libbsarchpp