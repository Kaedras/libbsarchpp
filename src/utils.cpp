#include "utils.h"
#include "types.h"    // for Magic4
#include <algorithm>  // for __transform_fn, transform, __replace_fn, replace
#include <cctype>     // for tolower, isalpha, isdigit, isascii
#include <cstring>    // for memcpy, size_t
#include <ranges>

using namespace std;

namespace {
inline constexpr unsigned char asciiDiff    = 'a' - 'A';
inline constexpr unsigned char extAsciiDiff = 0x20;

constexpr char toLowerWin1252(const char c) noexcept {
  const auto uc = static_cast<unsigned char>(c);
  // 'A' <= uc <= 'Z'
  if (uc >= 65 && uc <= 90) {
    return static_cast<char>(uc + asciiDiff);
  }

  // 'À' <= uc <= 'Ö' || 'Ù' <= uc <= 'Þ'
  if ((uc >= 0xC0 && uc <= 0xD6) || (uc >= 0xD8 && uc <= 0xDE)) {
    return static_cast<char>(uc + extAsciiDiff);
  }

  switch (uc) {
  case 0x8A:                         // Š
    return static_cast<char>(0x9A);  // š
  case 0x8C:                         // Œ
    return static_cast<char>(0x9C);  // œ
  case 0x8E:                         // Ž
    return static_cast<char>(0x9E);  // ž
  case 0x9F:                         // Ÿ
    return static_cast<char>(0xFF);  // ÿ
  default:
    return c;
  }
}
}  // namespace

namespace libbsarchpp {

uint32_t magicToInt(const Magic4 value) noexcept {
  uint32_t result;
  memcpy(&result, value.data(), 4);
  return result;
}

Magic4 intToMagic(const uint32_t value) noexcept {
  Magic4 result;
  memcpy(result.data(), &value, 4);
  return result;
}

Magic4 stringToMagic(const std::string& str) noexcept {
  Magic4 result = {0, 0, 0, 0};

  for (size_t i = 0; i < 4; i++) {
    if (str.length() > i) {
      result[i] = str[i];
    } else {
      break;
    }
  }
  return result;
}

std::string magicToString(const Magic4& magic) noexcept {
  string str(4, 0);
  memcpy(str.data(), magic.data(), 4);
  return str;
}

std::string magicToString(const uint32_t& value) noexcept {
  string str(4, 0);
  memcpy(str.data(), &value, 4);
  return str;
}

std::string toLower(const std::string_view str) noexcept {
  return str | std::views::transform(toLowerWin1252) | std::ranges::to<std::string>();
}

void toLowerInline(std::string& str) noexcept {
  ranges::transform(str, str.begin(), toLowerWin1252);
}

void normalizePath(std::string& str) noexcept {
  // replace '/' with '\\'
  ranges::replace(str, '/', '\\');
  // change string to lower characters
  ranges::transform(str, str.begin(), toLowerWin1252);
}

void changeSlashesToBackslashes(std::string& str) noexcept {
  for (auto& c : str) {
    if (c == '/') {
      c = '\\';
    }
  }
}

std::string_view getFileName(std::string_view str) {
  if (str == "/" || str.empty()) {
    return str;
  }

  const size_t lastSlash = str.find_last_of('/');
  if (lastSlash == string_view::npos) {
    return str;
  }
  return {str.data() + lastSlash + 1, str.length() - lastSlash - 1};
}

std::string_view getParentPath(std::string_view str) {
  if (str == "/" || str.empty()) {
    return str;
  }

  const size_t parentPathEnd   = str.find_last_of('/');
  const size_t parentPathStart = str.find_last_of('/', parentPathEnd - 1);

  return {str.data() + parentPathStart, parentPathEnd - parentPathStart};
}

bool sortPaths(const std::filesystem::path& lhs, const std::filesystem::path& rhs) noexcept {
  const string lhsStr = lhs.generic_string();
  const string rhsStr = rhs.generic_string();

  // approximate sorting order:
  // '/' == '\'
  // < ' '
  // < '!'
  // < '#'
  // < '$'
  // < '%'
  // < '&'
  // < '\''
  // < '('
  // < ')'
  // < '+'
  // < ','
  // < '-'
  // < '.'
  // < 0-9
  // < ';'
  // < '='
  // < '@'
  // < alphabetical
  // < non-ascii
  // < '['
  // < ']'
  // < '^'
  // < '_'
  // < '`'
  // < '{'
  // < '}'
  // < '~'
  // NOTE: '\'' < '.' < alphabetical, there may be characters in between
  // NOTE: '!' < "#" < '-', there may be characters in between
  // NOTE: '+' < '_', there may be characters in between

  for (unsigned long i = 0; i < lhsStr.length() && i < rhsStr.length(); i++) {
    const unsigned char& l = lhsStr[i];
    const unsigned char& r = rhsStr[i];

    // continue if both chars are identical
    if (tolower(l) == tolower(r)) {
      continue;
    }

    // if one char is '/' or '\\'
    if (l == '/' || l == '\\' || r == '/' || r == '\\') {
      return l == '/' || l == '\\';
    }
    // if one char is ' '
    if (l == ' ' || r == ' ') {
      return l == ' ';
    }
    // if one char is '!'
    if (l == '!' || r == '!') {
      return l == '!';
    }
    // if one char is '#'
    if (l == '#' || r == '#') {
      return l == '#';
    }
    // if one char is '$'
    if (l == '$' || r == '$') {
      return l == '$';
    }
    // if one char is '%'
    if (l == '%' || r == '%') {
      return l == '%';
    }
    // if one char is '&'
    if (l == '&' || r == '&') {
      return l == '&';
    }
    // if one char is '\''
    if (l == '\'' || r == '\'') {
      return l == '\'';
    }
    // if one char is '('
    if (l == '(' || r == '(') {
      return l == '(';
    }
    // if one char is ')'
    if (l == ')' || r == ')') {
      return l == ')';
    }
    // if one char is '+'
    if (l == '+' || r == '+') {
      return l == '+';
    }
    // if one char is ','
    if (l == ',' || r == ',') {
      return l == ',';
    }
    // if one char is '-'
    if (l == '-' || r == '-') {
      return l == '-';
    }
    // if one char is '.'
    if (l == '.' || r == '.') {
      return l == '.';
    }
    // if only one char is a digit
    if (static_cast<bool>(isdigit(l)) ^ static_cast<bool>(isdigit(r))) {
      return isdigit(l) != 0;
    }
    // if both chars are digits
    if (static_cast<bool>(isdigit(l)) && static_cast<bool>(isdigit(r))) {
      return l < r;
    }
    // if one char is ';'
    if (l == ';' || r == ';') {
      return l == ';';
    }
    // if one char is '='
    if (l == '=' || r == '=') {
      return l == '=';
    }
    // if one char is '@'
    if (l == '@' || r == '@') {
      return l == '@';
    }
    // if only one char is alphabetical
    if (static_cast<bool>(isalpha(l)) ^ static_cast<bool>(isalpha(r))) {
      return isalpha(l) != 0;
    }
    // if both chars are alphabetical
    if (static_cast<bool>(isalpha(l)) && static_cast<bool>(isalpha(r))) {
      return tolower(l) < tolower(r);
    }
    // if both chars are not ascii
    if (!static_cast<bool>(isascii(l)) && !static_cast<bool>(isascii(r))) {
      return l < r;
    }
    // if one char is '['
    if (l == '[' || r == '[') {
      return l == '[';
    }
    // if one char is ']'
    if (l == ']' || r == ']') {
      return l == ']';
    }
    // if one char is '^'
    if (l == '^' || r == '^') {
      return l == '^';
    }
    // if one char is '_'
    if (l == '_' || r == '_') {
      return l == '_';
    }
    // if one char is '`'
    if (l == '`' || r == '`') {
      return l == '`';
    }
    // if one char is '{'
    if (l == '{' || r == '{') {
      return l == '{';
    }
    // if one char is '}'
    if (l == '}' || r == '}') {
      return l == '}';
    }
    // if one char is '~'
    if (l == '~' || r == '~') {
      return l == '~';
    }

    // unhandled characters, just return l < r
    return l < r;
  }

  return lhsStr.length() < rhsStr.length();
}

}  // namespace libbsarchpp
