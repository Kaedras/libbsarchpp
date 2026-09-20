#pragma once

#include <array>        // for array
#include <cstdint>      // for uint8_t, uint32_t, uint64_t
#include <string_view>  // for string_view

// modified version of https://github.com/Zunawe/md5-c

using md5sum = std::array<uint8_t, 16>;

md5sum md5(const uint8_t* data, size_t length);
md5sum md5(std::string_view input);
