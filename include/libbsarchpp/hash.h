#pragma once

#include <cstdint>
#include <filesystem>

namespace libbsarchpp {
/**
 * @brief Creates file hash for Morrowind archives.
 */
uint64_t CreateHashTES3(const std::filesystem::path& fileName) noexcept;

/**
 * @brief Creates file hash for Oblivion archives.
 * @note Directories may contain '.', so a parameter is required to distinguish between files and directories.
 */
uint64_t CreateHashTES4(const std::filesystem::path& fileName, bool isDirectory) noexcept;

/**
 * @brief Creates file hash for FO4 archives.
 */
uint32_t CreateHashFO4(const std::filesystem::path& fileName) noexcept;
}  // namespace libbsarchpp