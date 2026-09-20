#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(SSE, BsaTest,
                         testing::Values(tuple<const char*, const char*>{"sse", "ccBGSSSE001-Fish.bsa"},
                                         tuple<const char*, const char*>{"sse", "ccBGSSSE025-AdvDSGS.bsa"},
                                         tuple<const char*, const char*>{"sse", "ccBGSSSE037-Curios.bsa"},
                                         tuple<const char*, const char*>{"sse", "ccQDRSSE001-SurvivalMode.bsa"},
                                         tuple<const char*, const char*>{"sse", "MarketplaceTextures.bsa"},
                                         tuple<const char*, const char*>{"sse", "_ResourcePack.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Animations.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Interface.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Meshes0.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Meshes1.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Misc.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Shaders.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Sounds.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures0.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures1.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures2.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures3.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures4.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures5.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures6.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures7.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Textures8.bsa"},
                                         tuple<const char*, const char*>{"sse", "Skyrim - Voices_en0.bsa"}));
