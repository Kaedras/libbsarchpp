#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(
    TES5, BsaTest,
    testing::Values(tuple<const char*, const char*>{"tes5", "Dawnguard.bsa"},
                    tuple<const char*, const char*>{"tes5", "Dragonborn.bsa"},
                    tuple<const char*, const char*>{"tes5", "HearthFires.bsa"},
                    tuple<const char*, const char*>{"tes5", "HighResTexturePack01.bsa"},
                    tuple<const char*, const char*>{
                        "tes5",
                        "HighResTexturePack02.bsa"},  // this archive must be compressed because it would be >4GiB
                    tuple<const char*, const char*>{"tes5", "HighResTexturePack03.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Animations.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Interface.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Meshes.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Misc.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Shaders.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Sounds.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Textures.bsa"},
                    tuple<const char*, const char*>{"tes5", "Skyrim - Voices.bsa"},
                    tuple<const char*, const char*>{"tes5", "Update.bsa"}));
