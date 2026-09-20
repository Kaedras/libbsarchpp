#include "Bsa.h"
#include "include/libbsarchpp/types.h"
#include "src/md5.h"

#include <algorithm>
#include <array>
#include <gsl-lite/gsl-lite.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace libbsarchpp;
namespace fs = std::filesystem;

namespace {
constexpr string_view testString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
                                   "incididunt ut labore et dolore magna aliqua.";
constexpr std::array<uint8_t, 16> md5{0X81, 0X8C, 0X6E, 0X60, 0X1A, 0X24, 0XF7, 0X27,
                                      0X50, 0XDA, 0X0F, 0X6C, 0X9B, 0X8E, 0XBE, 0X28};
}  // namespace

TEST(Misc, md5String) {
  EXPECT_EQ(md5String(testString), md5);
}

TEST(Misc, md5File) {
  // create file
  const auto filePath = fs::temp_directory_path() / "libbsarchpp_tests_md5File";
  filePtr file(fopen(filePath.c_str(), "w+"));
  ASSERT_NE(file, nullptr) << "error opening file " << filePath << ": " << strerror(errno);
  size_t result = fwrite(testString.data(), 1, testString.size(), file.get());
  ASSERT_EQ(result, testString.size());

  fseek(file.get(), 0, SEEK_SET);
  EXPECT_EQ(md5File(file.get()), md5);
}

TEST(Misc, GslThrowsexceptionOnNarrow) {
  EXPECT_THROW([[maybe_unused]] auto tmp = gsl_lite::narrow<int8_t>(200), gsl_lite::narrowing_error);
}
