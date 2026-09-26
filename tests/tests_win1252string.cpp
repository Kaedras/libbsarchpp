#include "src/win1252.h"

#include <gtest/gtest.h>

using namespace std;

TEST(Win1252, TES3) {
  const win1252string str{"/test/abc"s};

  EXPECT_EQ(str.parentPath(), "/test");
}
