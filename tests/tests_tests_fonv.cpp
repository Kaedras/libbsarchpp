#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(FNV, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"fnv", "CaravanPack - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "ClassicPack - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "DeadMoney - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "DeadMoney - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Meshes.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Misc.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Sound.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Textures2.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Textures.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Fallout - Voices1.bsa"},
                                         tuple<const char*, const char*>{"fnv", "GunRunnersArsenal - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "GunRunnersArsenal - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fnv", "HonestHearts - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "HonestHearts - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fnv", "LonesomeRoad - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "LonesomeRoad - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fnv", "MercenaryPack - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "OldWorldBlues - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "OldWorldBlues - Sounds.bsa"},
                                         tuple<const char*, const char*>{"fnv", "TribalPack - Main.bsa"},
                                         tuple<const char*, const char*>{"fnv", "Update.bsa"}));
