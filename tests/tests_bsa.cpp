#include "Bsa.h"
#include "gsl-lite/gsl-lite.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <src/utils.h>

using namespace std;
using namespace libbsarchpp;
namespace fs = std::filesystem;

TEST(Bsa, IterateFiles) {
  Bsa bsa("files/fo4dds.ba2");

  FileIterationFunction foo = [](const auto& a, auto, auto, auto f) {
    static_cast<vector<fs::path>*>(f)->emplace_back(a);
    return true;
  };

  vector<fs::path> files;

  bsa.iterateFiles(foo, &files);

  EXPECT_EQ(files.at(0).string(), "textures/grass/test.dds"s);
}

TEST(CreateArchive, FilesListEmpty) {
  vector<fs::path> filesList;
  EXPECT_THROW(Bsa("test.bsa", libbsarchpp::TES4, filesList), std::runtime_error);
}
