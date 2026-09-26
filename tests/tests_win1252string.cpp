#include "src/win1252.h"

#include <gtest/gtest.h>

using namespace std;

TEST(Win1252, filename) {
  const win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.filename(), "abc.xyz.wasd");
}

TEST(Win1252, extension) {
  const win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.extension(), ".wasd");
}

TEST(Win1252, stem) {
  const win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.stem(), "abc.xyz");
}
