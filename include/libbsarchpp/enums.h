#pragma once

#include <cstdio>
#include <string>

namespace libbsarchpp {
/**
 * Per file compression options
 */
enum class PackingCompression_t
{
    global,      /**< use global compression setting */
    compressed,  /**< compressed */
    uncompressed /**< uncompressed */
};

/**
 * Compression types for BSA files
 */
enum CompressionType
{
    zlib,     /**< zlib */
    lz4Frame, /**< lz4 frame */
    lz4Block  /**< lz4 block */
};

enum ArchiveType
{
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

inline std::string ToString(ArchiveType type)
{
    using std::string_literals::operator""s;
    switch (type)
    {
        case TES3: return "Morrowind"s;
        case TES4: return "Oblivion"s;
        case FO3: return "Skyrim LE, New Vegas, Fallout 3"s;
        case SSE: return "Skyrim SE, Skyrim AE"s;
        case FO4: return "Fallout 4"s;
        case FO4dds: return "Fallout 4 DDS"s;
        case SF: return "Starfield"s;
        case SFdds: return "Starfield DDS"s;
        case none:
        default: return "None"s;
    }
}

/**
 * Seek direction to use with @link Bsa::seek @endlink
 */
enum SeekDirection
{
    SET = SEEK_SET, /**< Seek from beginning of file. */
    CUR = SEEK_CUR, /**< Seek from current position. */
    END = SEEK_END  /**< Seek from end of file. */
};
} // namespace libbsarchpp