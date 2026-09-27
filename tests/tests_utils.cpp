#include <algorithm>
#include <gtest/gtest.h>
#include <src/utils.h>

using namespace std;
using namespace libbsarchpp;

TEST(Utils, magicToInt) {
  constexpr Magic4 btdx = {'B', 'T', 'D', 'X'};
  EXPECT_EQ(magicToInt(btdx), 0x58445442u);

  constexpr Magic4 bsa = {'B', 'S', 'A', '\0'};
  EXPECT_EQ(magicToInt(bsa), 0x00415342u);

  constexpr Magic4 zero = {0, 0, 0, 0};
  EXPECT_EQ(magicToInt(zero), 0u);

  constexpr Magic4 bytes = {0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(magicToInt(bytes), 0x04030201u);

  constexpr Magic4 allOnes = {static_cast<char>(0xFF), static_cast<char>(0xFF), static_cast<char>(0xFF),
                              static_cast<char>(0xFF)};
  EXPECT_EQ(magicToInt(allOnes), 0xFFFFFFFFu);
}

TEST(Utils, intToMagic) {
  constexpr Magic4 expectedBtdx = {'B', 'T', 'D', 'X'};
  EXPECT_EQ(intToMagic(0x58445442u), expectedBtdx);

  constexpr Magic4 expectedBsa = {'B', 'S', 'A', '\0'};
  EXPECT_EQ(intToMagic(0x00415342u), expectedBsa);

  constexpr Magic4 expectedZero = {0, 0, 0, 0};
  EXPECT_EQ(intToMagic(0u), expectedZero);

  constexpr Magic4 expectedBytes = {0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(intToMagic(0x04030201u), expectedBytes);

  // Round-trip tests
  constexpr uint32_t testVal = 0x12345678u;
  EXPECT_EQ(magicToInt(intToMagic(testVal)), testVal);

  constexpr Magic4 testMagic = {'T', 'E', 'S', '4'};
  EXPECT_EQ(intToMagic(magicToInt(testMagic)), testMagic);
}

TEST(Utils, stringToMagic) {
  constexpr Magic4 expectedBtdx = {'B', 'T', 'D', 'X'};
  EXPECT_EQ(stringToMagic("BTDX"), expectedBtdx);

  constexpr Magic4 expectedBsa = {'B', 'S', 'A', 0};
  EXPECT_EQ(stringToMagic("BSA"), expectedBsa);

  constexpr Magic4 expectedSingle = {'X', 0, 0, 0};
  EXPECT_EQ(stringToMagic("X"), expectedSingle);

  constexpr Magic4 expectedEmpty = {0, 0, 0, 0};
  EXPECT_EQ(stringToMagic(""), expectedEmpty);

  // Strings longer than 4 characters should be truncated
  EXPECT_EQ(stringToMagic("BTDX_EXTRA"), expectedBtdx);
  constexpr Magic4 expectedDigits = {'1', '2', '3', '4'};
  EXPECT_EQ(stringToMagic("123456789"), expectedDigits);
}

TEST(Utils, magicToString) {
  constexpr Magic4 btdx = {'B', 'T', 'D', 'X'};
  EXPECT_EQ(magicToString(btdx), "BTDX");

  constexpr Magic4 bsa = {'B', 'S', 'A', '\0'};
  EXPECT_EQ(magicToString(bsa), string("BSA\0", 4));

  constexpr Magic4 zeros = {0, 0, 0, 0};
  EXPECT_EQ(magicToString(zeros), string("\0\0\0\0", 4));

  // uint32_t overload
  EXPECT_EQ(magicToString(0x58445442u), "BTDX");
  EXPECT_EQ(magicToString(0x00415342u), string("BSA\0", 4));
  EXPECT_EQ(magicToString(0u), string("\0\0\0\0", 4));

  // Verify resulting string length is always 4
  EXPECT_EQ(magicToString(btdx).length(), 4u);
  EXPECT_EQ(magicToString(0x58445442u).length(), 4u);
}

TEST(Utils, toLower) {
  EXPECT_EQ(toLower("UPPER CASE"), "upper case");
  EXPECT_EQ(toLower("MixEd_CaSe-123!#"), "mixed_case-123!#");
  EXPECT_EQ(toLower("lower CASE 123"), "lower case 123");
  EXPECT_EQ(toLower(""), "");
  // Windows-1252 characters
  EXPECT_EQ(toLower("\xDC\xC4/\xDC\xDF"), "\xFC\xE4/\xFC\xDF");

  // latin-1 uppercase letters:
  const string win1252Latin1Upper =
      "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD8\xD9\xDA\xDB"
      "\xDC\xDD\xDE";
  const string win1252Latin1Lower =
      "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF8\xF9\xFA\xFB"
      "\xFC\xFD\xFE";
  EXPECT_EQ(toLower(win1252Latin1Upper), win1252Latin1Lower);

  EXPECT_EQ(toLower("\x8A\x8C\x8E\x9F"), "\x9A\x9C\x9E\xFF");

  // non-cased symbols
  const string symbols = "\xD7 \xF7 \x80 \xA9 \xAE \xA7 123 !@#";
  EXPECT_EQ(toLower(symbols), symbols);
}

TEST(Utils, toLowerInline) {
  string str1 = "UPPER CASE";
  toLowerInline(str1);
  EXPECT_EQ(str1, "upper case");

  string str2 = "MixEd_CaSe 123!@#";
  toLowerInline(str2);
  EXPECT_EQ(str2, "mixed_case 123!@#");

  string str3 = "lower case";
  toLowerInline(str3);
  EXPECT_EQ(str3, "lower case");

  string str4;
  toLowerInline(str4);
  EXPECT_EQ(str4, "");

  // Windows-1252
  string strWin1252 = "\xD6 \x8A\x8C\x8E\x9F \xC9\xC9\x45";
  toLowerInline(strWin1252);
  EXPECT_EQ(strWin1252, "\xF6 \x9A\x9C\x9E\xFF \xE9\xE9\x65");

  string strWin1252AlreadyLower = "already lower \xE4\xF6\xFC\xDF \x9A\x9C\x9E\xFF";
  toLowerInline(strWin1252AlreadyLower);
  EXPECT_EQ(strWin1252AlreadyLower, "already lower \xE4\xF6\xFC\xDF \x9A\x9C\x9E\xFF");
}

TEST(Utils, normalizePath) {
  string path1 = "Textures/SubFolder/TEX_Diffuse.DDS";
  normalizePath(path1);
  EXPECT_EQ(path1, "textures\\subfolder\\tex_diffuse.dds");

  string path2 = "MESHES\\ARMOR\\HELMET.NIF";
  normalizePath(path2);
  EXPECT_EQ(path2, "meshes\\armor\\helmet.nif");

  string path3 = "Mixed/Slashes\\And/BACK/Slashes";
  normalizePath(path3);
  EXPECT_EQ(path3, "mixed\\slashes\\and\\back\\slashes");

  string path4 = "NO_SLASHES_HERE.TXT";
  normalizePath(path4);
  EXPECT_EQ(path4, "no_slashes_here.txt");

  string path5;
  normalizePath(path5);
  EXPECT_EQ(path5, "");

  // Windows-1252 path normalization
  string winPath1 = "Textures/\xC4/\xD6\xDC\x8AP\xC9\x8E.DDS";
  normalizePath(winPath1);
  EXPECT_EQ(winPath1, "textures\\\xE4\\\xF6\xFC\x9Ap\xE9\x9E.dds");

  string winPath2 = "MESHES/\xCB/\x8C\x9FP\xDD.NIF";
  normalizePath(winPath2);
  EXPECT_EQ(winPath2, "meshes\\\xEB\\\x9C\xFFp\xFD.nif");

  string winPath3 = "SOUND/VOICE/\xC7/\xC2\xC7\xC0.WAV";
  normalizePath(winPath3);
  EXPECT_EQ(winPath3, "sound\\voice\\\xE7\\\xE2\xE7\xE0.wav");
}

TEST(Utils, changeSlashesToBackslashes) {
  string str1 = "textures/subfolder/file.dds";
  changeSlashesToBackslashes(str1);
  EXPECT_EQ(str1, "textures\\subfolder\\file.dds");

  string str2 = "meshes\\armor/helmet.nif";
  changeSlashesToBackslashes(str2);
  EXPECT_EQ(str2, "meshes\\armor\\helmet.nif");

  string str3 = "no_slashes_in_path";
  changeSlashesToBackslashes(str3);
  EXPECT_EQ(str3, "no_slashes_in_path");

  string str4;
  changeSlashesToBackslashes(str4);
  EXPECT_EQ(str4, "");

  string str5 = "///";
  changeSlashesToBackslashes(str5);
  EXPECT_EQ(str5, "\\\\\\");
}

TEST(Utils, getParentPath) {
  EXPECT_EQ(getParentPath("/abc/def"), "/abc");
  EXPECT_EQ(getParentPath("/abc/def/ghi"), "/abc/def");
  EXPECT_EQ(getParentPath("/"), "/");
  EXPECT_EQ(getParentPath(""), "");
}

TEST(Utils, getFileName) {
  EXPECT_EQ(getFileName("/abc/def"), "def");
  EXPECT_EQ(getFileName("/abc/def.test"), "def.test");
  EXPECT_EQ(getFileName("/"), "/");
  EXPECT_EQ(getFileName(""), "");
}

TEST(Utils, getFileExtension) {
  EXPECT_EQ(getFileExtension("/abc/test.txt"), ".txt");
  EXPECT_EQ(getFileExtension("/abc/test.tar.bz2"), ".bz2");
}

TEST(Utils, getFileStem) {
  EXPECT_EQ(getFileStem("/abc/test.txt"), "test");
  EXPECT_EQ(getFileStem("/abc/test.tar.xz"), "test.tar");
}

TEST(Utils, sortPaths) {
  vector<filesystem::path> paths = {
      ".", " ", "_", "-", ",", ";", "!", "'", "(", ")", "[", "]", "{",
      "}", "@", "&", "#", "%", "`", "^", "+", "=", "~", "$", "0", "a",
  };

  static vector<filesystem::path> target = {
      " ", "!", "#", "$", "%", "&", "'", "(", ")", "+", ",", "-", ".",
      "0", ";", "=", "@", "a", "[", "]", "^", "_", "`", "{", "}", "~",
  };

  ranges::sort(paths, sortPaths);

  ASSERT_EQ(paths.size(), target.size()) << "Paths and Target are of unequal length, " << paths.size() << ", "
                                         << target.size();

  for (size_t i = 0; i < paths.size(); i++) {
    SCOPED_TRACE(i);
    ASSERT_EQ(paths[i], target[i]);
  }
}
