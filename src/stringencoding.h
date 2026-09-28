#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

namespace libbsarchpp {

std::filesystem::path win1252ToPath(std::span<const uint8_t> data);
std::filesystem::path win1252ToPath(std::string_view data);
std::string pathToWin1252(const std::filesystem::path& path);
size_t getWin1252Length(const std::filesystem::path& path);

}  // namespace libbsarchpp
