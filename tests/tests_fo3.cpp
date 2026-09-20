#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(FO3, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"fo3", "Anchorage - Main.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Anchorage - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fo3", "BrokenSteel - Main.bsa"},
                                         tuple<const char*, const char*>{"fo3", "BrokenSteel - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - MenuVoices.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - Meshes.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - Misc.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - Sound.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - Textures.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Fallout - Voices.bsa"},
                                         tuple<const char*, const char*>{"fo3", "PointLookout - Main.bsa"},
                                         tuple<const char*, const char*>{"fo3", "PointLookout - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fo3", "ThePitt - Main.bsa"},
                                         tuple<const char*, const char*>{"fo3", "ThePitt - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Zeta - Main.bsa"},
                                         tuple<const char*, const char*>{"fo3", "Zeta - Sounds.bsa"}));
