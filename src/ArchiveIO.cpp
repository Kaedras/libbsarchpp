#include "ArchiveIO.h"

#include "utils.h"

#include <gsl-lite/gsl-lite.hpp>

using namespace std;
namespace fs = std::filesystem;
using gsl_lite::narrow;

namespace libbsarchpp {

namespace {
#ifdef __unix__
  // On Windows long is only 32-bit, so we can't use ftell there. On POSIX systems it's 64-bit
  int64_t _ftelli64(FILE* stream) {
    return ftell(stream);
  }

  // same with fseek
  int64_t _fseeki64(FILE* stream, long offset, int whence) {
    return fseek(stream, offset, whence);
  }
#endif

}  // namespace

int64_t ArchiveIO::tell() const {
  return _ftelli64(m_file.get());
}

void ArchiveIO::seek(int64_t pos, SeekDirection whence) const noexcept(false) {
  int64_t result = _fseeki64(m_file.get(), pos, whence);
  if (result != 0) {
    const int error = errno;
    throw runtime_error("Seek error: "s + strerror(error));
  }
}

ArchiveIO::ArchiveIO(std::filesystem::path file) : m_fileName(std::move(file)) {}

void ArchiveIO::open(std::filesystem::path file, mode m) {
  m_fileName = std::move(file);
  open(m);
}

void ArchiveIO::open(mode m) {
  const char* mode;
  if (m == mode::read) {
    mode = "rb";
  } else {
    mode = "wb";
  }

  m_file.reset(fopen(m_fileName.string().c_str(), mode));

  if (m_file == nullptr) {
    const int error = errno;
    throw runtime_error(format("Could not open file \"{}\" for reading: {}", m_fileName.string(), strerror(error)));
  }
}

bool ArchiveIO::isOpen() const {
  return m_file != nullptr;
}

}  // namespace libbsarchpp
