#pragma once

#include "../types.h"
#include "../dllexport.h"
#include <cstdint>
#include <filesystem>
#include <string>

namespace libbsarchpp {
[[nodiscard]] uint32_t MagicToInt(Magic4 value) noexcept;
[[nodiscard]] Magic4 IntToMagic(uint32_t value) noexcept;
[[nodiscard]] Magic4 StringToMagic(const std::string &str) noexcept;
[[nodiscard]] std::string MagicToString(const Magic4 &magic) noexcept;
[[nodiscard]] std::string MagicToString(const uint32_t &value) noexcept;

[[nodiscard]] std::string ToLower(std::string str) noexcept;
[[nodiscard]] std::string ToLower(const std::filesystem::path &str) noexcept;
void ToLowerInline(std::string &str) noexcept;

/**
 * @brief Replaces slashes with backslashes and changes string to lower case
 */
void normalizePath(std::string &str) noexcept;
void changeSlashesToBackslashes(std::u16string &str) noexcept;

/**
 * @brief This function is used to sort paths alphabetically.
 * @throw std::runtime_error If one string contains a character that is not accounted for
 */
[[nodiscard]] DLL_PUBLIC bool comparePaths(const std::filesystem::path &lhs, const std::filesystem::path &rhs) noexcept(false);
} // namespace libbsarchpp