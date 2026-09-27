#include "src/stringencoding.h"

#include <gtest/gtest.h>

using namespace std;
namespace fs = std::filesystem;
using namespace libbsarchpp;

TEST(StringEncoding, getWin1252Length) {
  EXPECT_EQ(getWin1252Length(fs::path(u8"/test/äbcdé")), 11);
  EXPECT_EQ(getWin1252Length(fs::path(u8"/×ÿß/®±¿¼")), 9);
}
