#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(TES4, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"tes4", "DLCBattlehornCastle.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCFrostcrag.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCHorseArmor.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCOrrery.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCShiveringIsles - Meshes.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCShiveringIsles - Sounds.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCShiveringIsles - Textures.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCShiveringIsles - Voices.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCThievesDen.bsa"},
                                         tuple<const char*, const char*>{"tes4", "DLCVileLair.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Knights.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Oblivion - Meshes.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Oblivion - Misc.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Oblivion - Sounds.bsa"},
                                         tuple<const char*, const char*>{"tes4",
                                                                         "Oblivion - Textures - Compressed.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Oblivion - Voices1.bsa"},
                                         tuple<const char*, const char*>{"tes4", "Oblivion - Voices2.bsa"}));
