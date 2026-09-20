#include "BsaTest.h"

using namespace std;

INSTANTIATE_TEST_SUITE_P(
    SF, BsaTest,
    testing::Values(
        tuple<const char*, const char*>{
            "sf", "Starfield - Meshes01.ba2"},  // extracting this file with xEdit will take several hours
        tuple<const char*, const char*>{"sf", "Starfield - Meshes02.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - MeshesPatch.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Misc.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Particles.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - PlanetData.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Shaders.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Terrain01.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Terrain02.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Terrain03.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Terrain04.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - TerrainPatch.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Voices01.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - Voices02.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - VoicesPatch.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSounds01.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSounds02.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSounds03.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSounds04.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSounds05.ba2"},
        tuple<const char*, const char*>{"sf", "Starfield - WwiseSoundsPatch.ba2"}));

INSTANTIATE_TEST_SUITE_P(
    SFTextures, BsaTest,
    testing::Values(tuple<const char*, const char*>{"sf", "Starfield - Textures01.ba2"},
                    // tuple<const char *, const char *>{"sf", "Starfield - Textures02.ba2"}, // packing this archive
                    // fails with "File packing error: Unsupported uncompressed DDS format"
                    // tuple<const char *, const char *>{"sf", "Starfield - Textures03.ba2"}, // packing this archive
                    // fails with "File packing error: Unsupported uncompressed DDS format"
                    tuple<const char*, const char*>{"sf", "Starfield - Textures04.ba2"},
                    // tuple<const char *, const char *>{"sf", "Starfield - Textures05.ba2"}, // packing this archive
                    // fails with "File packing error: Unsupported uncompressed DDS format"
                    tuple<const char*, const char*>{"sf", "Starfield - Textures06.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - Textures07.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - Textures08.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - Textures09.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - Textures10.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - Textures11.ba2"},
                    tuple<const char*, const char*>{"sf", "Starfield - TexturesPatch.ba2"}));
