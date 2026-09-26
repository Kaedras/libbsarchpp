#pragma once

#include "libbsarchpp/types.h"
#include <cstdint>
#include <filesystem>
#include <string>

namespace libbsarchpp {
[[nodiscard]] uint32_t magicToInt(Magic4 value) noexcept;
[[nodiscard]] Magic4 intToMagic(uint32_t value) noexcept;
[[nodiscard]] Magic4 stringToMagic(const std::string& str) noexcept;
[[nodiscard]] std::string magicToString(const Magic4& magic) noexcept;
[[nodiscard]] std::string magicToString(const uint32_t& value) noexcept;

[[nodiscard]] std::string toLower(std::string str) noexcept;
[[nodiscard]] std::string toLower(const std::filesystem::path& str) noexcept;
void toLowerInline(std::string& str) noexcept;

/**
 * @brief Replaces slashes with backslashes and changes string to lower case
 */
void normalizePath(std::string& str) noexcept;
void changeSlashesToBackslashes(std::string& str) noexcept;

std::string_view getFileName(std::string_view str);
std::string_view getParentPath(std::string_view str);

/**
 * @brief This function for use with `std::sort` to sort paths alphabetically.
 */
[[nodiscard]] bool sortPaths(const std::filesystem::path& lhs, const std::filesystem::path& rhs) noexcept;
}  // namespace libbsarchpp
