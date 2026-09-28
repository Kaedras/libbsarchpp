#pragma once

#include "libbsarchpp/enums.h"

#include <cstdint>
#include <cstdio>

namespace libbsarchpp {

/**
 * Compression types for BSA files
 */
enum CompressionType : uint8_t {
  zlib,     /**< zlib */
  lz4Frame, /**< lz4 frame */
  lz4Block  /**< lz4 block */
};

/**
 * Seek direction
 */
enum SeekDirection : uint8_t {
  SET = SEEK_SET, /**< Seek from beginning of file. */
  CUR = SEEK_CUR, /**< Seek from current position. */
  END = SEEK_END  /**< Seek from end of file. */
};

}  // namespace libbsarchpp
