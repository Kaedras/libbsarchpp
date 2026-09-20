#include "src/hash.h"

#include <gtest/gtest.h>

TEST(Hash, TES3) {
  EXPECT_EQ(libbsarchpp::CreateHashTES3("meshes/c/artifact_bloodring_01.nif"), 0x1C3C1149920D5F0C);
}

TEST(Hash, TES4) {
  EXPECT_EQ(libbsarchpp::CreateHashTES4("dlc2mq05__0003c745_1 .fuz", false),
            0x948228C0641531A0);  // TODO: figure out why this test fails on windows
  EXPECT_EQ(libbsarchpp::CreateHashTES4("interface\\controls\\ps3", true), 0x3DC5F43669167333);
  EXPECT_EQ(libbsarchpp::CreateHashTES4("interface\\controls\\360", true), 0x3DC5F3F969163630);
  EXPECT_EQ(libbsarchpp::CreateHashTES4("keyboard_english.txt", false), 0x1FDC44B06B107368);
  EXPECT_EQ(libbsarchpp::CreateHashTES4("gamepad.txt", false), 0x6CF7725967076164);
}

TEST(Hash, FO4) {
  EXPECT_EQ(libbsarchpp::CreateHashFO4("PipBoy01(Black)"), 0x1C70F7B2);
  EXPECT_EQ(libbsarchpp::CreateHashFO4("PIpbOY01(BLaCk)"), 0x1C70F7B2);
}
