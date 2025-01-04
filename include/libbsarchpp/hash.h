#pragma once

#include "dllexport.h"
#include <cstdint>
#include <filesystem>

namespace libbsarchpp {
/**
 * @brief Creates file hash for Morrowind archives.
 */
DLL_PUBLIC uint64_t CreateHashTES3(const std::filesystem::path &fileName) noexcept;

/**
 * @brief Creates file hash for Oblivion archives.
 * @note Directories may contain '.', so a parameter is required to distinguish between files and directories.
 */
DLL_PUBLIC uint64_t CreateHashTES4(const std::filesystem::path &fileName, bool isDirectory) noexcept;

/**
 * @brief Creates file hash for FO4 archives.
 */
DLL_PUBLIC uint32_t CreateHashFO4(const std::filesystem::path &fileName) noexcept;
} // namespace libbsarchpp