#pragma once

#include <string>
#include <unordered_map>

namespace checksums {
static const std::unordered_map<std::string, std::string> tes5_shared
    = {{"Dawnguard.bsa", "2ad45561e504f253c56da5d20814972e6caba23a1c2a9f794f8f70da49cc2f6e"},
       {"Dragonborn.bsa", "beb2ebd7ce5561d886e79ee15d46ef2ccf3732ceb837c746c9df1547351db2a4"},
       {"HearthFires.bsa", "1950a3a52dbd6ca8e1c480e3fca0a7d18cd7e61aaba90e8ff4955e015f6261b0"},
       {"HighResTexturePack01.bsa", "1aaf6075de4bc3de4247da211a49193159400c6f2fe732680ae867a3b4e04d6f"},
       {"HighResTexturePack02.bsa", "54f4b7d01ddb47e707ae760ac29b072b4d99c920fe98a1914e8f06996780316e"},
       {"HighResTexturePack03.bsa", "dd82a27d5ba17a37d2ea32f3c583cc50f41eb0fa1dec5bd6683221ebdc608dce"},
       {"Skyrim - Animations.bsa", "be7537cf2b8722f825501d92a83ae54462ddff7453fbd0c44109d01f51a1850c"},
       {"Skyrim - Interface.bsa", "0ccc8401035ccade0aba2960562ca08243077490bcdcefa113fd389a8adb5642"},
       {"Skyrim - Meshes.bsa", "366cd74bad7b95241cfe3cca66d82a339a02a515daf59f49de47f8277139ae00"},
       {"Skyrim - Misc.bsa", "a3e02920e6751acb1b40d48133b951861878215e419ecac527328044fff93cf2"},
       {"Skyrim - Shaders.bsa", "0cffe9fefa28517be481b4eb6fa992850944ed2b1d929c25d7590225fd642951"},
       {"Skyrim - Sounds.bsa", "0295efd56b2639047be3c782fb0839781f4ec38622e2db2f2ee40fc8753cff48"},
       {"Skyrim - Textures.bsa", "d32c54dafe38e7f41fe352f7b7a98875ea93c4e4e53cafd6c17b2f29c26919cb"},
       {"Skyrim - Voices.bsa", "daf3d98bab5e8314d109ac69c7d7d2e057a47387f742d0f4ce678f0299b2056c"},
       {"Update.bsa", "6b5c6311b7f5748d7cfee8d7735756d772afd149d0eb5119aab0975194c81b0b"}};
} // namespace checksums
