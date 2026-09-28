#pragma once

#include "libbsarchpp/types.h"
#include "md5.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>

#ifdef __GNUC__
#define PACKED(structure) structure __attribute__((__packed__))
#else
#define PACKED(structure) __pragma(pack(push, 1)) structure __pragma(pack(pop))
#endif

namespace libbsarchpp {

struct fileDeleter {
  void operator()(FILE* f) { fclose(f); }
};

using filePtr = std::unique_ptr<FILE, fileDeleter>;
using Magic4  = std::array<char, 4>;  // fourCC

// A string prefixed with a byte length. NOT null terminated.
struct bString {
  uint8_t length = 0;
  std::filesystem::path data;
};

// A string prefixed with a uint16 length. NOT null terminated.
struct wString {
  uint16_t length = 0;
  std::filesystem::path data;
};

PACKED(struct DDS_PIXELFORMAT {
  uint32_t size        = 0;
  uint32_t flags       = 0;
  uint32_t fourCC      = 0;
  uint32_t RGBBitCount = 0;
  uint32_t RBitMask    = 0;
  uint32_t GBitMask    = 0;
  uint32_t BBitMask    = 0;
  uint32_t ABitMask    = 0;
});
static_assert(sizeof(DDS_PIXELFORMAT) == 32);

#define RESERVED1_ARRAY std::array<uint32_t, 11>  // required to use multiple template arguments inside macro
PACKED(struct DDSHeader {
  uint32_t magic             = 0;
  uint32_t size              = 0;
  uint32_t flags             = 0;
  uint32_t height            = 0;
  uint32_t width             = 0;
  uint32_t pitchOrLinearSize = 0;
  uint32_t depth             = 0;
  uint32_t mipMapCount       = 0;
  RESERVED1_ARRAY reserved1  = {};
  DDS_PIXELFORMAT ddspf;

  uint32_t caps      = 0;
  uint32_t caps2     = 0;
  uint32_t caps3     = 0;
  uint32_t caps4     = 0;
  uint32_t reserved2 = 0;
});
static_assert(sizeof(DDSHeader) == 128);
#undef RESERVED1_ARRAY

PACKED(struct DDSHeaderDX10 {
  int32_t dxgiFormat         = 0;
  uint32_t resourceDimension = 0;
  uint32_t miscFlags         = 0;
  uint32_t arraySize         = 0;
  uint32_t miscFlags2        = 0;
});
static_assert(sizeof(DDSHeaderDX10) == 20);

struct DDSInfo {
  int32_t width   = 0;
  int32_t height  = 0;
  int32_t mipMaps = 0;
};

PACKED(struct HeaderTES3 {
  uint32_t hashOffset = 0;
  uint32_t fileCount  = 0;
});
static_assert(sizeof(HeaderTES3) == 8);

PACKED(struct HeaderTES4 {
  uint32_t foldersOffset     = 0;
  uint32_t flags             = 0;
  uint32_t folderCount       = 0;
  uint32_t fileCount         = 0;
  uint32_t folderNamesLength = 0;
  uint32_t fileNamesLength   = 0;
  uint32_t fileFlags         = 0;
});
static_assert(sizeof(HeaderTES4) == 28);

PACKED(struct HeaderFO4 {
  uint32_t magic          = 0;
  uint32_t fileCount      = 0;
  int64_t fileTableOffset = 0;
});
static_assert(sizeof(HeaderFO4) == 16);

PACKED(struct HeaderSF {
  HeaderFO4 fo4Header = {};
  uint32_t unknown1   = 0;
  uint32_t unknown2   = 0;
});
static_assert(sizeof(HeaderSF) == sizeof(HeaderFO4) + 8);

PACKED(struct HeaderSFdds {
  HeaderFO4 fo4Header        = {};
  uint32_t unknown1          = 0;
  uint32_t unknown2          = 0;
  uint32_t compressionMethod = 0;
});
static_assert(sizeof(HeaderSFdds) == sizeof(HeaderFO4) + 12);

struct PackedDataInfo {
  uint32_t size = 0;
  md5sum hash{};
  FileRecord_t fileRecord = nullptr;
};

}  // namespace libbsarchpp

#undef PACKED
