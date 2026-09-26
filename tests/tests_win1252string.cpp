#include "src/win1252string.h"

#include <gtest/gtest.h>

using namespace std;

TEST(Win1252string, filename) {
  const Win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.filename(), "abc.xyz.wasd");
}

TEST(Win1252string, extension) {
  const Win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.extension(), ".wasd");
}

TEST(Win1252string, stem) {
  const Win1252string str{"/test/abc.xyz.wasd"s};
  EXPECT_EQ(str.stem(), "abc.xyz");
}
