#include "Bsa.h"
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
constexpr md5sum expectedResult{0X81, 0X8C, 0X6E, 0X60, 0X1A, 0X24, 0XF7, 0X27,
                                0X50, 0XDA, 0X0F, 0X6C, 0X9B, 0X8E, 0XBE, 0X28};
}  // namespace

TEST(Misc, md5) {
  EXPECT_EQ(md5(testString), expectedResult);
  EXPECT_EQ(md5(reinterpret_cast<const uint8_t*>(testString.data()), testString.size()), expectedResult);
}

TEST(Misc, GslThrowsexceptionOnNarrow) {
  EXPECT_THROW([[maybe_unused]] auto tmp = gsl_lite::narrow<int8_t>(200), gsl_lite::narrowing_error);
}
