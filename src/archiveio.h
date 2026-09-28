#pragma once

#include "stringencoding.h"
#include "types.h"

#include <cstring>
#include <filesystem>

template <typename T, typename... Types>
inline constexpr bool is_none_of = (!std::same_as<T, Types> && ...);

namespace libbsarchpp {
using std::string_literals::operator""s;

class ArchiveIO {
public:
  enum class mode : uint8_t { read, write };

  ArchiveIO() = default;
  explicit ArchiveIO(std::filesystem::path file) noexcept;

  void open(std::filesystem::path file, mode m);
  void open(mode m);

  [[nodiscard]] bool isOpen() const noexcept;

  void setType(ArchiveType t) noexcept {
    m_type           = t;
    m_useBackslashes = m_type != FO4 && m_type != FO4dds && m_type != SF && m_type != SFdds;
  }

  [[nodiscard]] ArchiveType getType() const noexcept { return m_type; }

  [[nodiscard]] int64_t tell() const;

  /**
   * @brief Calls fseek and checks for errors.
   * @param pos Position to seek to
   * @param whence Seek direction
   * @throw std::runtime_error
   */
  void seek(int64_t pos, SeekDirection whence = SET) const noexcept(false);

  /**
   * @brief Reads data.
   * @tparam T Data type to read
   * @return Read value
   * @throw std::runtime_error
   */
  template <typename T>
  T read() noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    T retVal;

    if (fread(&retVal, sizeof(T), 1, m_file.get()) == 1) {
      return retVal;
    }

    if (feof(m_file.get())) {
      throw std::runtime_error("Read error: EOF");
    }

    throw std::runtime_error("Read error: "s + strerror(errno));
  }

  /**
   * @brief Reads an array.
   * @param length array length
   * @return std::vector<@p T> containing the read data
   * @throw std::runtime_error
   */
  template <typename T>
  std::vector<T> read(uint32_t length) noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    std::vector<T> retVal(length);

    if (fread(retVal.data(), sizeof(T), length, m_file.get()) == length) {
      return retVal;
    }

    const int error = errno;
    if (feof(m_file.get())) {
      throw std::runtime_error("Read error: EOF");
    }
    throw std::runtime_error("Read error: "s + strerror(error));
  }

  /**
   * @brief Reads an array from the open file into an existing array. Memory must already be allocated.
   * @param data Array to write into
   * @param length Elements to read
   * @throw std::runtime_error
   */
  template <typename T>
  void read(T* data, uint32_t length) noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    size_t result = fread(data, sizeof(T), length, m_file.get());
    if (result != length) {
      const int error = errno;
      if (feof(m_file.get())) {
        throw std::runtime_error("Read error: EOF");
      }
      throw std::runtime_error("Read error: "s + strerror(error));
    }
  }

  /**
   * @brief Writes data.
   * @throw std::runtime_error
   */
  template <typename T>
  void write(const T& data) noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    if (fwrite(&data, sizeof(T), 1, m_file.get()) != 1) {
      throw std::runtime_error("Write error: "s + strerror(errno));
    }
  }

  /**
   * @copybrief write(const T&)
   * @throw std::runtime_error
   */
  template <typename T>
  void write(const T* data, size_t length) noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    if (fwrite(data, sizeof(T), length, m_file.get()) != length) {
      throw std::runtime_error("Write error: "s + strerror(errno));
    }
  }

  /**
   * @copybrief write(const T&)
   * @throw std::runtime_error
   */
  template <typename T>
  void write(const std::vector<T>& data) noexcept(false) {
    static_assert(is_none_of<T, bString, bzString, wString, std::string, std::filesystem::path>);

    if (fwrite(data.data(), sizeof(T), data.size(), m_file.get()) != data.size()) {
      throw std::runtime_error("Write error: "s + strerror(errno));
    }
  }

private:
  std::unique_ptr<FILE, fileDeleter> m_file;
  std::filesystem::path m_fileName;
  ArchiveType m_type    = none;
  bool m_useBackslashes = false;

  std::filesystem::path readPath(size_t length);
};

template <>
inline bString ArchiveIO::read() noexcept(false) {
  auto length = read<uint8_t>();
  return {length, readPath(length)};
}

template <>
inline bzString ArchiveIO::read() noexcept(false) {
  auto length = read<uint8_t>();
  return {length, readPath(length)};
}

template <>
inline wString ArchiveIO::read() noexcept(false) {
  auto length = read<uint16_t>();
  return {length, readPath(length)};
}

template <>
inline zString ArchiveIO::read() noexcept(false) {
  zString str;
  char c = 1;
  while (c != 0) {
    c = read<char>();
    if (!isascii(c)) {
      throw std::runtime_error(std::to_string(c) + " is not an ASCII character");
    }
    str += c;
  }
  str.pop_back();
  return str;
}

template <>
inline std::filesystem::path ArchiveIO::read() noexcept(false) {
  std::vector<uint8_t> data;
  uint8_t value = 1;
  while (value != 0) {
    value = read<uint8_t>();
    data.push_back(value);
  }
  data.pop_back();
  return win1252ToPath(data);
}

template <>
inline void ArchiveIO::write(const std::filesystem::path& data) noexcept(false) {
  auto str = pathToWin1252(data);
  for (uint8_t c : str) {
    if (c == '/' && m_useBackslashes) {
      c = '\\';
    }
    write(c);
  }
}

template <>
inline void ArchiveIO::write(const bString& data) noexcept(false) {
  write(data.length);
  write(data.data);
}

template <>
inline void ArchiveIO::write(const bzString& data) noexcept(false) {
  write(data.length);
  write(data.data);
  write('\0');
}

template <>
inline void ArchiveIO::write(const wString& data) noexcept(false) {
  write(data.length);
  write(data.data);
}

template <>
inline void ArchiveIO::write(const zString& data) noexcept(false) {
  for (char c : data) {
    if (c == '/' && m_useBackslashes) {
      c = '\\';
    }
    write(c);
  }
  write('\0');
}

}  // namespace libbsarchpp
