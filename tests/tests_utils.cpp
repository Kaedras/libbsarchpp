#include <algorithm>
#include <gtest/gtest.h>
#include <src/utils.h>

using namespace std;
using namespace libbsarchpp;

TEST(Utils, Sorting) {
  // unused paths: "Ħ", "犬", "ß", "ü", "Ü", "\"", "?", ":", "|", "<", ">", "*", "§", "®", "Ø", "\\", "/", "ä", "Ä",
  // "ö", "Ö", "A",
  vector<filesystem::path> paths = {
    ".", " ", "_", "-", ",", ";", "!", "'", "(", ")", "[", "]", "{", "}",
    "@", "&", "#", "%", "`", "^", "+", "=", "~", "$", "¥", "0", "a", "ふ",
};

  static vector<filesystem::path> target = {" ", "!", "#", "$", "%", "&", "'", "(", ")", "+", ",", "-", ".", "0",
                                    ";", "=", "@", "a", "[", "]", "^", "_", "`", "{", "}", "~", "¥", "ふ"};

  ranges::sort(paths, comparePaths);

  ASSERT_EQ(paths.size(), target.size()) << "Paths and Target are of unequal length, " << paths.size() << ", "
                                         << target.size();

  for (size_t i = 0; i < paths.size(); i++) {
    SCOPED_TRACE(i);
    ASSERT_EQ(paths[i], target[i]);
  }
}
