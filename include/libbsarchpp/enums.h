#pragma once

#include <cstdint>
#include <string>

namespace libbsarchpp {
/**
 * Per file compression options
 */
enum class PackingCompression_t : uint8_t {
  global,      /**< use global compression setting */
  compressed,  /**< compressed */
  uncompressed /**< uncompressed */
};

enum ArchiveType : uint8_t {
  none,   /**< None */
  TES3,   /**< Morrowind */
  TES4,   /**< Oblivion */
  FO3,    /**< Skyrim LE, New Vegas, Fallout 3 */
  SSE,    /**< Skyrim SE, Skyrim AE */
  FO4,    /**< Fallout 4 */
  FO4dds, /**< Fallout 4 DDS */
  SF,     /**< Starfield */
  SFdds   /**< Starfield DDS */
};

namespace ArchiveFlag {
  inline constexpr uint32_t pathNames  = 0x0001;  // Include Directory Names. This bit is set in all official BSA files.
  inline constexpr uint32_t fileNames  = 0x0002;  // Include File Names. This bit is set in all official BSA files.
  inline constexpr uint32_t compress   = 0x0004;  // Compressed Archive.
  inline constexpr uint32_t retainDir  = 0x0008;  // Retain Directory Names.
  inline constexpr uint32_t retainName = 0x0010;  // Retain File Names.
  inline constexpr uint32_t retainOff  = 0x0020;  // Retain File Name Offsets.
  inline constexpr uint32_t xbox360    = 0x0040;  // Xbox360 archive.
  inline constexpr uint32_t startupStr = 0x0080;  // Retain Strings During Startup.
  inline constexpr uint32_t embedName  = 0x0100;  // File data blocks begin with a string containing the full file path.
  inline constexpr uint32_t xmem       = 0x0200;  // XMem Codec. This is an Xbox 360 only compression algorithm.
  inline constexpr uint32_t unknown10  = 0x0400;
}  // namespace ArchiveFlag

namespace FileFlag {
  inline constexpr uint32_t nif  = 0x0001;
  inline constexpr uint32_t dds  = 0x0002;
  inline constexpr uint32_t xml  = 0x0004;
  inline constexpr uint32_t wav  = 0x0008;
  inline constexpr uint32_t mp3  = 0x0010;
  inline constexpr uint32_t txt  = 0x0020;  // TXT, HTML, BAT, SCC
  inline constexpr uint32_t spt  = 0x0040;
  inline constexpr uint32_t fnt  = 0x0080;  // TEX, FNT
  inline constexpr uint32_t misc = 0x0100;  // CTL and others

  inline constexpr uint32_t compressed = 0x40000000;  // Whether the file is compressed
}  // namespace FileFlag

enum class HeaderVersion : uint8_t {
  TES4    = 0x67,  // Oblivion
  FO3     = 0x68,  // FO3, FNV, TES5
  SSE     = 0x69,  // SSE
  FO4v1   = 0x01,  // FO4
  SF      = 0x02,  // SF
  SFdds   = 0x03,  // SFdds
  FO4NGv7 = 0x07,  // FO4NG
  FO4NGv8 = 0x08   // FO4NG2
};

inline std::string toString(ArchiveType type) {
  using std::string_literals::operator""s;
  switch (type) {
  case TES3:
    return "Morrowind"s;
  case TES4:
    return "Oblivion"s;
  case FO3:
    return "Skyrim LE, New Vegas, Fallout 3"s;
  case SSE:
    return "Skyrim SE, Skyrim AE"s;
  case FO4:
    return "Fallout 4"s;
  case FO4dds:
    return "Fallout 4 DDS"s;
  case SF:
    return "Starfield"s;
  case SFdds:
    return "Starfield DDS"s;
  case none:
  default:
    return "None"s;
  }
}

}  // namespace libbsarchpp
