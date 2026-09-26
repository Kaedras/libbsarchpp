#include "Bsa.h"
#include "BsaTest.h"
#include "checksums/sha256sums.h"
#include "settings.h"

#include <fstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/types.h>

using namespace std;
using namespace libbsarchpp;

namespace {
void clean(const std::string& game) {
  std::filesystem::remove_all(WORKDIR / game);
}

std::string sha256(const std::filesystem::path& file) {
  // check if file exists
  if (!exists(file)) {
    throw runtime_error(format("[{}] file {} does not exist", __FUNCTION__, file.string()));
  }

  ifstream input(file, ios::binary);

  EVP_MD_CTX* mdCtx;
  unsigned char* digest;
  unsigned int digestLength;
  mdCtx = EVP_MD_CTX_new();
  if (mdCtx == nullptr) {
    throw runtime_error("EVP_MD_CTX_new error");
  }

  // initialize
  if (1 != EVP_DigestInit_ex(mdCtx, EVP_sha256(), nullptr)) {
    throw runtime_error("EVP_DigestInit_ex error");
  }

  constexpr size_t bufferSize{1u << 12u};
  std::vector buffer(bufferSize, '\0');

  while (input.good()) {
    input.read(buffer.data(), bufferSize);
    if (1 != EVP_DigestUpdate(mdCtx, buffer.data(), input.gcount())) {
      throw runtime_error("EVP_DigestUpdate error");
    }
  }

  // allocate memory
  digest = static_cast<unsigned char*>(OPENSSL_malloc(EVP_MD_size(EVP_sha256())));
  if (digest == nullptr) {
    throw runtime_error("OPENSSL_malloc error");
  }

  // finalize data
  if (1 != EVP_DigestFinal_ex(mdCtx, digest, &digestLength)) {
    OPENSSL_free(digest);
    throw runtime_error("EVP_DigestFinal_ex error");
  }
  EVP_MD_CTX_free(mdCtx);

  stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << hex << setw(2) << setfill('0') << static_cast<int>(digest[i]);
  }
  OPENSSL_free(digest);
  return ss.str();
}

void pack(const std::string& game, const std::filesystem::path& fileName, ArchiveType type, bool compressed,
          bool shared) {
  Bsa::create(WORKDIR / game / fileName, type, WORKDIR / game / fileName.stem(),
              {.multithreaded = multithreadedPacking, .compressed = compressed, .shareData = shared});
}

void extract(const std::string& game, const std::filesystem::path& fileName) {
  Bsa::extract(dataDirs[game] / fileName, WORKDIR / game / fileName.stem(), multithreadedExtracting);
}

ArchiveType getType(const std::string& game, const std::string& name) {
  return Bsa::getArchiveType(dataDirs[game] / name);
}

testing::AssertionResult ChecksumMatches(const std::filesystem::path& file, const std::string& checksum) {
  string fileChecksum = sha256(file);
  if (fileChecksum == checksum) {
    return testing::AssertionSuccess();
  }
  return testing::AssertionFailure() << file.filename().string() << " (size " << file_size(file)
                                     << ") has incorrect checksum: " + fileChecksum;
}

void recreateArchive(const std::string& game, const std::string& archive, bool compressed, bool shared) {
  // this directory is deleted after running the test, so we require it to be empty to prevent deletion of unrelated
  // files
  if (exists(WORKDIR / game)) {
    assert(std::filesystem::is_empty(WORKDIR / game));
  }

  // extract
  EXPECT_NO_THROW(extract(game, archive));
  // pack
  // this is a special case: the created archive would be >4GiB
  if (game == "tes5" && archive == "HighResTexturePack02.bsa" && !(compressed || shared)) {
    EXPECT_THROW(pack(game, archive, getType(game, archive), compressed, shared), runtime_error);
    // clean up
    clean(game);
    return;
  }
  EXPECT_NO_THROW(pack(game, archive, getType(game, archive), compressed, shared));

  // validate checksum
  string checksumName = game;
  if (compressed) {
    checksumName += "_compressed";
  } else if (shared) {
    checksumName += "_shared";
  }
  EXPECT_TRUE(ChecksumMatches(WORKDIR / game / archive, checksums::sha256sums.at(checksumName).at(archive)));
  // clean up
  clean(game);
}
}  // namespace

// prevent reporting an error when no game-specific tests are enabled
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BsaTest);

TEST_P(BsaTest, RecreateArchive) {
  string game    = get<0>(GetParam());
  string archive = get<1>(GetParam());

  recreateArchive(game, archive, false, false);
}

TEST_P(BsaTest, RecreateArchiveCompressed) {
  string game    = get<0>(GetParam());
  string archive = get<1>(GetParam());

  recreateArchive(game, archive, true, false);
}

TEST_P(BsaTest, RecreateArchiveShared) {
  string game    = get<0>(GetParam());
  string archive = get<1>(GetParam());

  recreateArchive(game, archive, false, true);
}
