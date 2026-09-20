#pragma once

#include "enums.h"
#include "libbsarchpp/Bsa.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <tuple>

class BsaTest : public testing::TestWithParam<std::tuple<const char*, const char*>> {
public:
  static void recreateArchive(const std::string& game, const std::string& archive, bool compressed, bool shared);

private:
  static void clean(const std::string& game);
  static std::string sha256(const std::filesystem::path& file);
  static void pack(const std::string& game, const std::filesystem::path& fileName, libbsarchpp::ArchiveType type,
                   bool compressed, bool shared);
  static void extract(const std::string& game, const std::filesystem::path& fileName);
  static libbsarchpp::ArchiveType getType(const std::string& game, const std::string& name);
  static testing::AssertionResult ChecksumMatches(const std::filesystem::path& file, const std::string& checksum);
};
