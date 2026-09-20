#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(SKYRIMVR, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"skyrimvr", "Skyrim - Animations.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Interface.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Meshes0.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Meshes1.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Misc.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Patch.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Shaders.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Sounds.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures0.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures1.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures2.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures3.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures4.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures5.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures6.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures7.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Textures8.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim - Voices_en0.bsa"},
                                         tuple<const char*, const char*>{"skyrimvr", "Skyrim_VR - Main.bsa"}));
