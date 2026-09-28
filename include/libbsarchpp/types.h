#pragma once

#include "enums.h"  // for PackingCompression_t

#include <array>        // for array
#include <cstdint>      // for uint32_t, uint16_t, uint64_t, uint8_t
#include <filesystem>   // for path
#include <string>       // for basic_string, string
#include <string_view>  // for string_view
#include <variant>      // for variant
#include <vector>       // for vector

namespace libbsarchpp {

// forward declarations
struct TexChunkRec;
struct FileTES3;
struct FileTES4;
struct FileFO4;

using FileRecord_t = std::variant<std::nullptr_t, FileTES3*, FileTES4*, FileFO4*, TexChunkRec*>;
using FilePtr_t    = std::variant<std::nullptr_t, FileTES3*, FileTES4*, FileFO4*>;

using Buffer = std::vector<uint8_t>;

// we want to be able to directly read and write structs, so we have to disable padding
#ifdef __GNUC__
#define PACKED(structure) structure __attribute__((__packed__))
#else
#define PACKED(structure) __pragma(pack(push, 1)) structure __pragma(pack(pop))
#endif

// A string prefixed with a byte length and null terminated
struct bzString {
  bzString& operator=(const std::filesystem::path& path);
  uint8_t length = 0;
  std::filesystem::path data;
};

struct FileTES3 {
  uint64_t hash   = 0;
  uint32_t size   = 0;
  uint32_t offset = 0;
  std::filesystem::path name;
};

struct FileTES4 {
  uint64_t hash   = 0;
  uint32_t size   = 0;
  uint32_t offset = 0;
  std::filesystem::path name;
  PackingCompression_t packingCompression = PackingCompression_t::global;
  [[nodiscard]] bool compress(bool globalCompress) const noexcept;  // compress when packing into a new archive
};

struct FolderTES4 {
  uint64_t hash      = 0;
  uint32_t fileCount = 0;
  uint32_t unk32     = 0;
  uint64_t offset    = 0;
  bzString name;
  std::vector<FileTES4> files;
};

PACKED(struct TexChunkRec {
  int64_t offset      = 0;
  uint32_t packedSize = 0;
  uint32_t size       = 0;
  uint16_t startMip   = 0;
  uint16_t endMip     = 0;
});
static_assert(sizeof(TexChunkRec) == 20);

struct FileFO4 {
  uint32_t nameHash = 0;
  std::array<char, 4> ext{0, 0, 0, 0};
  uint32_t dirHash = 0;
  // GNRL archive format
  uint32_t unknown    = 0;
  int64_t offset      = 0;
  uint32_t packedSize = 0;
  uint32_t size       = 0;
  // DX10 archive format
  uint8_t unknownTex = 0;
  // uint16_t chunkHeaderSize; this value is a constant (24)
  uint16_t height    = 0;
  uint16_t width     = 0;
  uint8_t numMips    = 0;
  uint8_t dxgiFormat = 0;
  uint16_t cubeMaps  = 0;
  std::vector<TexChunkRec> texChunks;

  std::filesystem::path name;
  PackingCompression_t packingCompression = PackingCompression_t::global;

  [[nodiscard]] std::string_view dxgiFormatName() const noexcept;
  [[nodiscard]] bool compress(bool globalCompress) const noexcept;  // compress when packing into a new archive
};

/**
 * Advanced settings for bsa creation
 */
struct BsaCreationSettings {
  /**
   * Enable multithreading. Increases performance but produces indeterministic results.
   */
  bool multithreaded = false;
  /**
   * Enable compression. Reduces file size and performance.
   */
  bool compressed      = false;
  int compressionLevel = 0;
  /**
   * Identical files will only be written once. May reduce filesize and may either reduce or increase performance.
   */
  bool shareData                = false;
  bool ignoreExtensionlessFiles = true; /**< don't add files without extensions to the archive */
  std::vector<std::string> extensionBlacklist{".exe", ".bsa", ".ba2", ".db"};
};
}  // namespace libbsarchpp

#undef PACKED
