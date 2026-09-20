#include "hash.h"

#include "constants.h"            // for crc32table
#include "gsl-lite/gsl-lite.hpp"  // for narrow
#include "utils.h"                // for normalizePath
#include <array>                  // for array
#include <climits>                // for CHAR_MAX, UCHAR_MAX
#include <cstddef>                // for size_t
#include <string>                 // for basic_string, operator==, char_traits

using namespace std;

namespace libbsarchpp {

uint64_t CreateHashTES3(const std::filesystem::path& fileName) noexcept {
  string s = fileName.string();
  normalizePath(s);

  uint32_t sum    = 0;
  uint32_t offset = 0;
  const size_t l  = s.length() >> 1u;

  for (size_t i = 0; i < l; i++) {
    const uint32_t tmp = static_cast<unsigned char>(s[i]) << (offset & 0x1Fu);
    sum ^= tmp;
    offset += 8;
  }
  const uint64_t result = static_cast<uint64_t>(sum) << 32u;

  sum    = 0;
  offset = 0;
  for (size_t i = l; i < s.length(); i++) {
    const uint32_t tmp = static_cast<unsigned char>(s[i]) << (offset & 0x1Fu);
    sum ^= tmp;
    const uint32_t n = tmp & 0x1Fu;
    sum              = (sum >> n) | (sum << (32 - n));
    offset += 8;
  }

  return result | sum;
}

uint64_t CreateHashTES4(const std::filesystem::path& fileName, const bool isDirectory) noexcept {
  try {
    uint32_t hash = 0;

    u16string name;
    string ext;

    if (fileName.has_extension() && !isDirectory) {
      name = fileName.stem().u16string();
      ext  = fileName.extension().string();
    } else {
      name = fileName.u16string();
    }

    const auto length = gsl_lite::narrow<uint32_t>(name.length());

    for (uint32_t i = 0; i < length; i++) {
      if (name[i] == '/') {
        name[i] = '\\';
      } else if (name[i] >= 'A' && name[i] <= 'Z') {
        name[i] = name[i] + ('a' - 'A');
      }
    }

    if (length == 0) {
      return 0;
    }

    uint64_t result = static_cast<uint8_t>(name[length - 1]);
    if (length > 2) {
      result |= static_cast<uint32_t>(name[length - 2] << 8u);
    }
    result |= length << 16u;
    result |= static_cast<uint32_t>(name[0] << 24u);

    if (ext == ".kf") {
      result |= 0x80u;
    } else if (ext == ".nif") {
      result |= 0x8000u;
    } else if (ext == ".dds") {
      result |= 0x8080u;
    } else if (ext == ".wav") {
      result |= 0x80000000u;
    }
    if (length > 1) {
      for (uint32_t i = 1; i < length - 2; i++) {
        hash = name[i] + (hash << 6u) + (hash << 16u) - hash;
      }
      result += static_cast<uint64_t>(hash) << 32u;
    }
    hash = 0;

    for (const char c : ext) {
      hash = c + (hash << 6u) + (hash << 16u) - hash;
    }

    result += static_cast<uint64_t>(hash) << 32u;

    return result;
  } catch (...) {
    return 0;
  }
}

uint32_t CreateHashFO4(const std::filesystem::path& fileName) noexcept {
  uint32_t result = 0;
  auto str        = fileName.string();

  normalizePath(str);

  for (const unsigned char c : str) {
    if (c <= CHAR_MAX) {
      result = (result >> 8u) ^ crc32table.at((result ^ c) & UCHAR_MAX);
    }
  }
  return result;
}

}  // namespace libbsarchpp
