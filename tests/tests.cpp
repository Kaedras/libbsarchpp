#include "checksums/sha256sums.h"
#include "libbsarchpp/Bsa.h"
#include "libbsarchpp/enums.h"
#include "libbsarchpp/hash.h"
#include "libbsarchpp/utils/utils.h"
#include "settings.h"

#include <bits/ranges_algo.h>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <gsl-lite/gsl-lite.hpp>
#include <gtest/gtest.h>
#include <iomanip>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/types.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace {

using namespace std;
using namespace libbsarchpp;
namespace fs = std::filesystem;

class BsaTest : public testing::TestWithParam<tuple<const char *, const char *>>
{
    // You can implement all the usual fixture class members here.
    // To access the test parameter, call GetParam() from class
    // TestWithParam<T>.
};

void clean(const string &game)
{
    remove_all(WORKDIR / game);
}

string sha256(const std::filesystem::path &file)
{
    // check if file exists
    if (!exists(file))
    {
        throw runtime_error(format("[{}] file {} does not exist", __FUNCTION__, file.string()));
    }

    ifstream input(file, ios::binary);

    EVP_MD_CTX *mdCtx;
    unsigned char *digest;
    unsigned int digestLength;
    mdCtx = EVP_MD_CTX_new();
    if (mdCtx == nullptr)
    {
        throw runtime_error("EVP_MD_CTX_new error");
    }

    // initialize
    if (1 != EVP_DigestInit_ex(mdCtx, EVP_sha256(), nullptr))
    {
        throw runtime_error("EVP_DigestInit_ex error");
    }

    constexpr size_t bufferSize{1 << 12};
    std::vector buffer(bufferSize, '\0');

    while (input.good())
    {
        input.read(buffer.data(), bufferSize);
        if (1 != EVP_DigestUpdate(mdCtx, buffer.data(), input.gcount()))
        {
            throw runtime_error("EVP_DigestUpdate error");
        }
    }

    // allocate memory
    digest = static_cast<unsigned char *>(OPENSSL_malloc(EVP_MD_size(EVP_sha256())));
    if (digest == nullptr)
    {
        throw runtime_error("OPENSSL_malloc error");
    }

    // finalize data
    if (1 != EVP_DigestFinal_ex(mdCtx, digest, &digestLength))
    {
        throw runtime_error("EVP_DigestFinal_ex error");
    }
    EVP_MD_CTX_free(mdCtx);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(digest[i]);
    }
    OPENSSL_free(digest);
    return ss.str();
}

void pack(const string &game, const fs::path &fileName, const ArchiveType type, bool compressed, bool shared)
{
    try
    {
        Bsa::create(WORKDIR / game / fileName,
                    type,
                    WORKDIR / game / fileName.stem(),
                    {.multithreaded = multithreadedPacking, .compressed = compressed, .shareData = shared});
    }
    catch (...)
    {
        throw;
    }
}

void extract(const string &game, const fs::path &fileName)
{
    Bsa::extract(dataDirs[game] / fileName, WORKDIR / game / fileName.stem(), multithreadedExtracting);
}

ArchiveType getType(const string &game, const string &name)
{
    return Bsa::getArchiveType(dataDirs[game] / name);
}

testing::AssertionResult ChecksumMatches(const fs::path &file, const std::string &checksum)
{
    string fileChecksum = sha256(file);
    if (fileChecksum == checksum)
    {
        return testing::AssertionSuccess();
    }
    return testing::AssertionFailure() << file.filename().string() << " (size " << file_size(file)
                                       << ") has incorrect checksum: " + fileChecksum;
}

TEST(Misc, Sorting)
{
    // unused paths: "Ħ", "犬", "ß", "ü", "Ü", "\"", "?", ":", "|", "<", ">", "*", "§", "®", "Ø", "\\", "/", "ä", "Ä", "ö", "Ö", "A",
    vector<fs::path> paths = {
        ".", " ", "_", "-", ",", ";", "!", "'", "(", ")", "[", "]", "{", "}",
        "@", "&", "#", "%", "`", "^", "+", "=", "~", "$", "¥", "0", "a", "ふ",
    };

    static vector<fs::path> target = {" ", "!", "#", "$", "%", "&", "'", "(", ")", "+", ",", "-", ".", "0",
                                      ";", "=", "@", "a", "[", "]", "^", "_", "`", "{", "}", "~", "¥", "ふ"};

    ranges::sort(paths, comparePaths);

    ASSERT_EQ(paths.size(), target.size())
        << "Paths and Target are of unequal length, " << paths.size() << ", " << target.size();

    for (size_t i = 0; i < paths.size(); i++)
    {
        SCOPED_TRACE(i);
        ASSERT_EQ(paths[i], target[i]);
    }
}

TEST(Misc, IterateFiles)
{
    Bsa bsa("files/fo4dds.ba2");

    FileIterationFunction foo = [](const auto &a, auto, auto, auto f) {
        static_cast<vector<fs::path> *>(f)->emplace_back(a);
        return true;
    };

    vector<fs::path> files;

    bsa.iterateFiles(foo, &files);

    EXPECT_EQ(files.at(0).string(), "textures/grass/test.dds"s);
}

TEST(GSL, exceptionOnNarrow)
{
    EXPECT_THROW([[maybe_unused]] auto tmp = gsl_lite::narrow<int8_t>(200), gsl_lite::narrowing_error);
}

TEST(Hash, TES3)
{
    EXPECT_EQ(libbsarchpp::CreateHashTES3("meshes/c/artifact_bloodring_01.nif"), 0x1C3C1149920D5F0C);
}
TEST(Hash, TES4)
{
    EXPECT_EQ(libbsarchpp::CreateHashTES4(u"dlc2mq05__0003c745_1 .fuz", false),
              0x948228C0641531A0); // TODO: figure out why this test fails on windows
    EXPECT_EQ(libbsarchpp::CreateHashTES4("interface\\controls\\ps3", true), 0x3DC5F43669167333);
    EXPECT_EQ(libbsarchpp::CreateHashTES4("interface\\controls\\360", true), 0x3DC5F3F969163630);
    EXPECT_EQ(libbsarchpp::CreateHashTES4("keyboard_english.txt", false), 0x1FDC44B06B107368);
    EXPECT_EQ(libbsarchpp::CreateHashTES4("gamepad.txt", false), 0x6CF7725967076164);
}
TEST(Hash, FO4)
{
    EXPECT_EQ(CreateHashFO4("PipBoy01(Black)"), 0x1C70F7B2);
    EXPECT_EQ(CreateHashFO4("PIpbOY01(BLaCk)"), 0x1C70F7B2);
}

TEST(CreateArchive, FilesListEmpty)
{
    vector<fs::path> filesList;
    EXPECT_THROW(Bsa("test.bsa", libbsarchpp::TES4, filesList), std::runtime_error);
}

/* the archive files for these tests have not been properly created yet
void run(const string &file, ArchiveType type, const char *checksum)
{
    // this directory is deleted after running the test, so we require it to be empty to prevent deletion of unrelated files
    if (exists(WORKDIR))
    {
        assert(fs::is_empty(WORKDIR));
    }
    // extract
    EXPECT_NO_THROW(Bsa::extract("files/"s + file, WORKDIR, true));
    // pack
    EXPECT_NO_THROW(Bsa::create(WORKDIR / file, type, WORKDIR));
    // validate checksum
    EXPECT_TRUE(ChecksumMatches(WORKDIR / file, checksum));
    // clean up
    remove_all(WORKDIR);
}

TEST(Fast, fo3)
{
    run("fo3.bsa", FO3, "8bb44e30c74ced406ab8bf04b62c4599a4278a1a2796b2b4dc9150de693749d8");
}
TEST(Fast, fo4)
{
    run("fo4.ba2", FO4, "3d04ce8ca656ec67a1532123daccef6ba64b6c463795e726157f4e6c301a296f");
}
TEST(Fast, fo4dds)
{
    run("fo4dds.ba2", FO4dds, "7ca461b6ccc01bd9dd56d8c6757b32b48ffe9574874ad16e23cabc8c80c07939");
}
TEST(Fast, sf)
{
    run("sf.ba2", SF, "1179e11e9cd2e5811f1138d313195f10de5ede311981a37eb331c2b445319bc3");
}
TEST(Fast, sse)
{
    run("sse.bsa", SSE, "ad00f7024debdfcfc0af56bd0ac0fbf0f9fbda40505e7d34387d7bf5d3952f1c");
}
TEST(Fast, tes3)
{
    run("tes3.bsa", TES3, "a0e18e10542ba4f3dae7bb59c710f4c8467ea174a553c46568c8f71bf0c9d59b");
}
TEST(Fast, tes4)
{
    run("tes4.bsa", TES4, "28508a8760bb662a5a4aeba2521bdfbe689cb601b376ba03816387a89b7219c8");
}
*/

void run(const string &game, const string &archive, bool compressed, bool shared)
{
    // this directory is deleted after running the test, so we require it to be empty to prevent deletion of unrelated files
    if (exists(WORKDIR / game))
    {
        assert(fs::is_empty(WORKDIR / game));
    }

    // extract
    EXPECT_NO_THROW(extract(game, archive));
    // pack
    // this is a special case: the created archive would be >4GiB
    if (game == "tes5" && archive == "HighResTexturePack02.bsa" && !(compressed || shared))
    {
        EXPECT_THROW(pack(game, archive, getType(game, archive), compressed, shared), runtime_error);
        // clean up
        clean(game);
        return;
    }
    EXPECT_NO_THROW(pack(game, archive, getType(game, archive), compressed, shared));

    // validate checksum
    string checksumName = game;
    if (compressed)
    {
        checksumName += "_compressed";
    }
    else if (shared)
    {
        checksumName += "_shared";
    }
    EXPECT_TRUE(ChecksumMatches(WORKDIR / game / archive, checksums::sha256sums.at(checksumName).at(archive)));
    // clean up
    clean(game);
}

TEST_P(BsaTest, RecreateArchive)
{
    string game = get<0>(GetParam());
    string archive = get<1>(GetParam());

    run(game, archive, false, false);
}

TEST_P(BsaTest, RecreateArchiveCompressed)
{
    string game = get<0>(GetParam());
    string archive = get<1>(GetParam());

    run(game, archive, true, false);
}

TEST_P(BsaTest, RecreateArchiveShared)
{
    string game = get<0>(GetParam());
    string archive = get<1>(GetParam());

    run(game, archive, false, true);
}

// Morrowind
#ifdef TESTS_TES3_ENABLE
INSTANTIATE_TEST_SUITE_P(TES3,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"tes3", "Bloodmoon.bsa"},
                                         tuple<const char *, const char *>{"tes3", "Morrowind.bsa"},
                                         tuple<const char *, const char *>{"tes3", "Tribunal.bsa"}));
#endif

// Oblivion
#ifdef TESTS_TES4_ENABLE
INSTANTIATE_TEST_SUITE_P(TES4,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"tes4", "DLCBattlehornCastle.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCFrostcrag.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCHorseArmor.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCOrrery.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCShiveringIsles - Meshes.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCShiveringIsles - Sounds.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCShiveringIsles - Textures.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCShiveringIsles - Voices.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCThievesDen.bsa"},
                                         tuple<const char *, const char *>{"tes4", "DLCVileLair.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Knights.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Oblivion - Meshes.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Oblivion - Misc.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Oblivion - Sounds.bsa"},
                                         tuple<const char *, const char *>{"tes4",
                                                                           "Oblivion - Textures - Compressed.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Oblivion - Voices1.bsa"},
                                         tuple<const char *, const char *>{"tes4", "Oblivion - Voices2.bsa"}));
#endif

// Skyrim
#ifdef TESTS_TES5_ENABLE
INSTANTIATE_TEST_SUITE_P(
    TES5,
    BsaTest,
    testing::Values(tuple<const char *, const char *>{"tes5", "Dawnguard.bsa"},
                    tuple<const char *, const char *>{"tes5", "Dragonborn.bsa"},
                    tuple<const char *, const char *>{"tes5", "HearthFires.bsa"},
                    tuple<const char *, const char *>{"tes5", "HighResTexturePack01.bsa"},
                    tuple<const char *, const char *>{
                        "tes5", "HighResTexturePack02.bsa"}, // this archive must be compressed because it would be >4GiB
                    tuple<const char *, const char *>{"tes5", "HighResTexturePack03.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Animations.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Interface.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Meshes.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Misc.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Shaders.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Sounds.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Textures.bsa"},
                    tuple<const char *, const char *>{"tes5", "Skyrim - Voices.bsa"},
                    tuple<const char *, const char *>{"tes5", "Update.bsa"}));
#endif

// Skyrim SE
#ifdef TESTS_SSE_ENABLE
INSTANTIATE_TEST_SUITE_P(SSE,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"sse", "ccBGSSSE001-Fish.bsa"},
                                         tuple<const char *, const char *>{"sse", "ccBGSSSE025-AdvDSGS.bsa"},
                                         tuple<const char *, const char *>{"sse", "ccBGSSSE037-Curios.bsa"},
                                         tuple<const char *, const char *>{"sse", "ccQDRSSE001-SurvivalMode.bsa"},
                                         tuple<const char *, const char *>{"sse", "MarketplaceTextures.bsa"},
                                         tuple<const char *, const char *>{"sse", "_ResourcePack.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Animations.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Interface.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Meshes0.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Meshes1.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Misc.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Shaders.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Sounds.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures0.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures1.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures2.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures3.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures4.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures5.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures6.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures7.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Textures8.bsa"},
                                         tuple<const char *, const char *>{"sse", "Skyrim - Voices_en0.bsa"}));
#endif

// Skyrim VR
#ifdef TESTS_SKYRIMVR_ENABLE
INSTANTIATE_TEST_SUITE_P(SKYRIMVR,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"skyrimvr", "Skyrim - Animations.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Interface.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Meshes0.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Meshes1.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Misc.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Patch.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Shaders.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Sounds.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures0.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures1.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures2.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures3.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures4.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures5.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures6.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures7.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Textures8.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim - Voices_en0.bsa"},
                                         tuple<const char *, const char *>{"skyrimvr", "Skyrim_VR - Main.bsa"}));
#endif

// Fallout 3
#ifdef TESTS_FO3_ENABLE
INSTANTIATE_TEST_SUITE_P(FO3,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"fo3", "Anchorage - Main.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Anchorage - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fo3", "BrokenSteel - Main.bsa"},
                                         tuple<const char *, const char *>{"fo3", "BrokenSteel - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - MenuVoices.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - Meshes.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - Misc.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - Sound.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - Textures.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Fallout - Voices.bsa"},
                                         tuple<const char *, const char *>{"fo3", "PointLookout - Main.bsa"},
                                         tuple<const char *, const char *>{"fo3", "PointLookout - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fo3", "ThePitt - Main.bsa"},
                                         tuple<const char *, const char *>{"fo3", "ThePitt - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Zeta - Main.bsa"},
                                         tuple<const char *, const char *>{"fo3", "Zeta - Sounds.bsa"}));
#endif

// Fallout: New Vegas
#ifdef TESTS_FONV_ENABLE
INSTANTIATE_TEST_SUITE_P(FNV,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"fnv", "CaravanPack - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "ClassicPack - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "DeadMoney - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "DeadMoney - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Meshes.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Misc.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Sound.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Textures2.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Textures.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Fallout - Voices1.bsa"},
                                         tuple<const char *, const char *>{"fnv", "GunRunnersArsenal - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "GunRunnersArsenal - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fnv", "HonestHearts - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "HonestHearts - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fnv", "LonesomeRoad - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "LonesomeRoad - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fnv", "MercenaryPack - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "OldWorldBlues - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "OldWorldBlues - Sounds.bsa"},
                                         tuple<const char *, const char *>{"fnv", "TribalPack - Main.bsa"},
                                         tuple<const char *, const char *>{"fnv", "Update.bsa"}));
#endif

// Fallout 4
#ifdef TESTS_FO4_ENABLE
INSTANTIATE_TEST_SUITE_P(
    FO4,
    BsaTest,
    testing::Values(tuple<const char *, const char *>{"fo4", "ccBGSFO4001-PipBoy(Black) - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4003-PipBoy(Camo01) - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4004-PipBoy(Camo02) - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4006-PipBoy(Chrome) - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4016-Prey - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4018-GaussRiflePrototype - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4019-ChineseStealthArmor - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4020-PowerArmorSkin(Black) - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4038-HorseArmor - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4044-HellfirePowerArmor - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4046-TesCan - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4096-AS_Enclave - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4110-WS_Enclave - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4115-X02 - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4116-HeavyFlamer - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFRSFO4001-HandmadeShotgun - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4001-ModularMilitaryBackpack - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4002-MidCenturyModern - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4007-Halloween - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccOTMFO4001-Remnants - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccSBJFO4003-Grenade - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCCoast - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCCoast - Voices_en.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCNukaWorld - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCNukaWorld - Voices_en.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCRobot - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCRobot - Voices_en.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop01 - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop02 - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop03 - Main.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop03 - Voices_en.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Animations.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Interface.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Materials.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Meshes.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - MeshesExtra.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Misc.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Nvflex.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Shaders.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Sounds.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Startup.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Voices.ba2"}));

// Fallout 4 textures
INSTANTIATE_TEST_SUITE_P(
    FO4Textures,
    BsaTest,
    testing::Values(tuple<const char *, const char *>{"fo4", "ccBGSFO4001-PipBoy(Black) - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4003-PipBoy(Camo01) - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4004-PipBoy(Camo02) - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4006-PipBoy(Chrome) - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4016-Prey - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4018-GaussRiflePrototype - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4019-ChineseStealthArmor - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4020-PowerArmorSkin(Black) - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4038-HorseArmor - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4044-HellfirePowerArmor - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4046-TesCan - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4096-AS_Enclave - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4110-WS_Enclave - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4115-X02 - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccBGSFO4116-HeavyFlamer - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFRSFO4001-HandmadeShotgun - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4001-ModularMilitaryBackpack - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4002-MidCenturyModern - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccFSVFO4007-Halloween - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccOTMFO4001-Remnants - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "ccSBJFO4003-Grenade - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCCoast - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCNukaWorld - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCRobot - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop01 - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop02 - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "DLCworkshop03 - Textures.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures1.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures2.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures3.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures4.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures5.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures6.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures7.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures8.ba2"},
                    tuple<const char *, const char *>{"fo4", "Fallout4 - Textures9.ba2"}));
#endif

// Fallout 4 VR
#ifdef TESTS_FO4VR_ENABLE
INSTANTIATE_TEST_SUITE_P(FO4VR,
                         BsaTest,
                         testing::Values(tuple<const char *, const char *>{"fo4vr", "Fallout4 - Animations.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Interface.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Materials.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Meshes.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - MeshesExtra.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Misc.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Misc - Beta.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Misc - Debug.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Shaders.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Sounds.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Startup.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures1.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures2.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures3.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures4.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures5.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures6.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures7.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures8.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Textures9.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4 - Voices.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4_VR - Main.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4_VR - Shaders.ba2"},
                                         tuple<const char *, const char *>{"fo4vr", "Fallout4_VR - Textures.ba2"}));
#endif

// Starfield
#ifdef TESTS_SF_ENABLE
INSTANTIATE_TEST_SUITE_P(SF,
                         BsaTest,
                         testing::Values(
                             tuple<const char *, const char *>{
                                 "sf",
                                 "Starfield - Meshes01.ba2"}, // extracting this file with xEdit will take several hours
                             tuple<const char *, const char *>{"sf", "Starfield - Meshes02.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - MeshesPatch.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Misc.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Particles.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - PlanetData.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Shaders.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Terrain01.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Terrain02.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Terrain03.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Terrain04.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - TerrainPatch.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Voices01.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - Voices02.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - VoicesPatch.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSounds01.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSounds02.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSounds03.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSounds04.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSounds05.ba2"},
                             tuple<const char *, const char *>{"sf", "Starfield - WwiseSoundsPatch.ba2"}));

// Starfield Textures
INSTANTIATE_TEST_SUITE_P(
    SFTextures,
    BsaTest,
    testing::Values(
        tuple<const char *, const char *>{"sf", "Starfield - Textures01.ba2"},
        // tuple<const char *, const char *>{"sf", "Starfield - Textures02.ba2"}, // packing this archive fails with "File packing error: Unsupported uncompressed DDS format"
        // tuple<const char *, const char *>{"sf", "Starfield - Textures03.ba2"}, // packing this archive fails with "File packing error: Unsupported uncompressed DDS format"
        tuple<const char *, const char *>{"sf", "Starfield - Textures04.ba2"},
        // tuple<const char *, const char *>{"sf", "Starfield - Textures05.ba2"}, // packing this archive fails with "File packing error: Unsupported uncompressed DDS format"
        tuple<const char *, const char *>{"sf", "Starfield - Textures06.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - Textures07.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - Textures08.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - Textures09.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - Textures10.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - Textures11.ba2"},
        tuple<const char *, const char *>{"sf", "Starfield - TexturesPatch.ba2"}));

#endif

}