#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(TES3, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"tes3", "Bloodmoon.bsa"},
                                         tuple<const char*, const char*>{"tes3", "Morrowind.bsa"},
                                         tuple<const char*, const char*>{"tes3", "Tribunal.bsa"}));