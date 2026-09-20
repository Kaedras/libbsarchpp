#include "Bsa.h"
#include "gsl-lite/gsl-lite.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <src/utils.h>

using namespace std;
using namespace libbsarchpp;
namespace fs = std::filesystem;

TEST(Misc, GslThrowsexceptionOnNarrow) {
  EXPECT_THROW([[maybe_unused]] auto tmp = gsl_lite::narrow<int8_t>(200), gsl_lite::narrowing_error);
}
