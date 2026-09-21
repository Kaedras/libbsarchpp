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

template <>
char ArchiveIO::read() noexcept(false) {
  int result = fgetc(m_file.get());
  if (result == EOF) {
    const int error = errno;
    throw runtime_error("Read error: "s + strerror(error));
  }
  return static_cast<char>(result);
}

template <>
char8_t ArchiveIO::read() noexcept(false) {
  int result = fgetc(m_file.get());
  if (result == EOF) {
    const int error = errno;
    throw runtime_error("Read error: "s + strerror(error));
  }
  return static_cast<char8_t>(result);
}

template <>
Magic4 ArchiveIO::read() noexcept(false) {
  Magic4 result{};
  size_t itemsRead = fread(result.data(), 1, 4, m_file.get());
  if (itemsRead != 4) {
    const int error = errno;
    throw runtime_error("Read error: "s + strerror(error));
  }
  return result;
}

template <>
std::filesystem::path ArchiveIO::read<>() noexcept(false) {
  try {
    u16string str;

    char16_t c;
    do {
      c = read<uint8_t>();

      // we use forward slashes internally, this is required for std::filesystem
      if (c == '\\') {
        c = '/';
      }

      str += c;
    } while (c != '\0');

    str.pop_back();

    return {str};
  } catch (...) {
    throw;
  }
}

std::u16string ArchiveIO::readU16String(const uint32_t length) noexcept(false) {
  try {
    u16string str;

    for (uint32_t i = 0; i < length; i++) {
      char16_t c = read<uint8_t>();

      // we use forward slashes internally, this is required for std::filesystem
      if (c == '\\') {
        c = '/';
      }
      str += c;
    }

    return str;
  } catch (...) {
    throw;
  }
}

std::filesystem::path ArchiveIO::readStringLen(const bool terminated) noexcept(false) {
  try {
    const auto length = read<uint8_t>();
    auto str          = readU16String(length);
    if (terminated) {
      str.pop_back();
    }
    return {str};
  } catch (...) {
    throw;
  }
}

std::filesystem::path ArchiveIO::readStringLen16() noexcept(false) {
  try {
    const auto length = read<uint16_t>();
    auto str          = readU16String(length);
    return {str};
  } catch (...) {
    throw;
  }
}

template <>
void ArchiveIO::write(const std::string& data) noexcept(false) {
  try {
    for (char c : data) {
      if (c == '/' && m_useBackslashes) {
        c = '\\';
      }
      write(c);
    }
    if (fputc('\0', m_file.get()) == EOF) {
      const int error = errno;
      throw runtime_error("Write error: "s + strerror(error));
    }
  } catch (...) {
    throw;
  }
}

template <>
void ArchiveIO::write(const std::filesystem::path& data) noexcept(false) {
  try {
    u16string str = data.generic_u16string();

    // we use forward slashes internally, so we have to change them when writing
    if (m_useBackslashes) {
      changeSlashesToBackslashes(str);
    }

    for (const auto& c : str) {
      if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF) {
        const int error = errno;
        throw runtime_error("Read error: "s + strerror(error));
      }
    }
    if (fputc('\0', m_file.get()) == EOF) {
      const int error = errno;
      throw runtime_error("Read error: "s + strerror(error));
    }
  } catch (const gsl_lite::narrowing_error& ex) {
    throw runtime_error(ex.what());
  }
}

void ArchiveIO::writeStringLen8(const std::filesystem::path& data, const bool terminated) noexcept(false) {
  try {
    u16string str = data.generic_u16string();
    auto length   = gsl_lite::narrow<uint8_t>(str.length());
    if (terminated) {
      length++;
    }
    write(length);

    // we use forward slashes internally, so we have to change them when writing
    if (m_useBackslashes) {
      changeSlashesToBackslashes(str);
    }

    for (const auto& c : str) {
      if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF) {
        const int error = errno;
        throw runtime_error("Error writing to file: "s + strerror(error));
      }
    }
    if (terminated) {
      if (fputc('\0', m_file.get()) == EOF) {
        const int error = errno;
        throw runtime_error("Error writing to file: "s + strerror(error));
      }
    }
  } catch (const gsl_lite::narrowing_error& ex) {
    throw runtime_error(ex.what());
  } catch (...) {
    throw;
  }
}

void ArchiveIO::writeStringLen16(const std::filesystem::path& data) noexcept(false) {
  try {
    u16string str = data.generic_u16string();
    write(gsl_lite::narrow<uint16_t>(str.length()));

    // we use forward slashes internally, so we have to change them when writing
    if (m_useBackslashes) {
      changeSlashesToBackslashes(str);
    }

    for (const auto& c : str) {
      if (fputc(gsl_lite::narrow<uint8_t>(c), m_file.get()) == EOF) {
        const int error = errno;
        throw runtime_error("Write error: "s + strerror(error));
      }
    }
  } catch (const gsl_lite::narrowing_error& ex) {
    throw runtime_error(ex.what());
  } catch (...) {
    throw;
  }
}

int64_t ArchiveIO::tell() const {
  return _ftelli64(m_file.get());
}

int64_t ArchiveIO::seek(int64_t pos, SeekDirection whence) const noexcept(false) {
  int64_t result = _fseeki64(m_file.get(), pos, whence);
  if (result != 0) {
    const int error = errno;
    throw runtime_error("Seek error: "s + strerror(error));
  }

  return result;
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
