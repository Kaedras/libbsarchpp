#pragma once

#include "types.h"

#include <cstring>
#include <filesystem>

namespace libbsarchpp {
using std::string_literals::operator""s;

class ArchiveIO {
public:
  enum class mode : uint8_t { read, write };

  ArchiveIO() = default;
  explicit ArchiveIO(std::filesystem::path file);

  void open(std::filesystem::path file, mode m);
  void open(mode m);

  [[nodiscard]] bool isOpen() const;

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
  int64_t seek(int64_t pos, SeekDirection whence = SET) const noexcept(false);

  /**
   * @brief Reads data.
   * @tparam T Data type to read
   * @return Read value
   * @throw std::runtime_error
   */
  template <typename T>
  T read() noexcept(false) {
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
   * @brief Reads an u16string.
   * @param length String length
   * @throw std::runtime_error
   */
  std::u16string readU16String(uint32_t length) noexcept(false);

  /**
   * @brief Reads a string that starts with an uint8_t indicating the length.
   * @param terminated Whether string is null terminated
   * @throw std::runtime_error
   */
  std::filesystem::path readStringLen(bool terminated = true) noexcept(false);

  /**
   * @brief Reads a file name that starts with an uint16_t indicating the length.
   * @throw std::runtime_error
   */
  std::filesystem::path readStringLen16() noexcept(false);

  /**
   * @brief Writes data.
   * @throw std::runtime_error
   */
  template <typename T>
  void write(const T& data) noexcept(false) {
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
    if (fwrite(data.data(), sizeof(T), data.size(), m_file.get()) != data.size()) {
      throw std::runtime_error("Write error: "s + strerror(errno));
    }
  }

  /**
   * @brief Writes a string that starts with an uint8_t indicating the length.
   * @param data String to write
   * @param terminated Write null-terminated string
   * @throw std::runtime_error
   */
  void writeStringLen8(const std::filesystem::path& data, bool terminated = true) noexcept(false);

  /**
   * @brief Writes a string that starts with an uint16_t indicating the length.
   * @throw std::runtime_error
   */
  void writeStringLen16(const std::filesystem::path& data) noexcept(false);

private:
  std::unique_ptr<FILE, fileDeleter> m_file;
  std::filesystem::path m_fileName;
  ArchiveType m_type    = none;
  bool m_useBackslashes = false;
};

}  // namespace libbsarchpp
