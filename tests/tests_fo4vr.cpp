#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(FO4VR, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"fo4vr", "Fallout4 - Animations.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Interface.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Materials.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Meshes.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - MeshesExtra.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Misc.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Misc - Beta.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Misc - Debug.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Shaders.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Sounds.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Startup.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures1.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures2.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures3.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures4.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures5.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures6.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures7.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures8.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Textures9.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4 - Voices.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4_VR - Main.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4_VR - Shaders.ba2"},
                                         tuple<const char*, const char*>{"fo4vr", "Fallout4_VR - Textures.ba2"}));
