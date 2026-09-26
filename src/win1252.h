#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief A Windows-1252 encoded file path
 */
class win1252string {
public:
  win1252string() = default;

  explicit win1252string(std::string rawData);
  explicit win1252string(const std::vector<uint8_t>& rawData);
  explicit win1252string(std::u8string_view u8Str);
  explicit win1252string(const std::filesystem::path& path);

  win1252string& operator=(std::string str);

  [[nodiscard]] std::u8string toU8String() const;
  [[nodiscard]] std::string toUtf8() const;
  [[nodiscard]] std::string_view parentPath() const;

  [[nodiscard]] const std::string& string() const noexcept { return m_data; }
  [[nodiscard]] const char* data() const noexcept { return m_data.data(); }
  [[nodiscard]] size_t length() const { return m_data.size(); }

  /**
   * @brief Replaces slashes with backslashes and changes string to lower case
   */
  void normalizePath();
  [[nodiscard]] std::string extension() const;
  [[nodiscard]] std::string stem() const;

private:
  std::string m_data;

  void normalizeSeparators() noexcept {
    for (char& c : m_data) {
      if (c == '\\') {
        c = '/';
      }
    }
  }
};
