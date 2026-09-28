#include "stringencoding.h"

#include <algorithm>
#include <array>
#include <cstring>

using namespace std;

namespace {

inline constexpr array<u8string_view, 256> win1252ToUtf8 = {
    u8"\x00", u8"\x01", u8"\x02", u8"\x03", u8"\x04", u8"\x05", u8"\x06",   u8"\x07", u8"\x08", u8"\x09", u8"\x0A",
    u8"\x0B", u8"\x0C", u8"\x0D", u8"\x0E", u8"\x0F", u8"\x10", u8"\x11",   u8"\x12", u8"\x13", u8"\x14", u8"\x15",
    u8"\x16", u8"\x17", u8"\x18", u8"\x19", u8"\x1A", u8"\x1B", u8"\x1C",   u8"\x1D", u8"\x1E", u8"\x1F", u8" ",
    u8"!",    u8"\"",   u8"#",    u8"$",    u8"%",    u8"&",    u8"'",      u8"(",    u8")",    u8"*",    u8"+",
    u8",",    u8"-",    u8".",    u8"/",    u8"0",    u8"1",    u8"2",      u8"3",    u8"4",    u8"5",    u8"6",
    u8"7",    u8"8",    u8"9",    u8":",    u8";",    u8"<",    u8"=",      u8">",    u8"?",    u8"@",    u8"A",
    u8"B",    u8"C",    u8"D",    u8"E",    u8"F",    u8"G",    u8"H",      u8"I",    u8"J",    u8"K",    u8"L",
    u8"M",    u8"N",    u8"O",    u8"P",    u8"Q",    u8"R",    u8"S",      u8"T",    u8"U",    u8"V",    u8"W",
    u8"X",    u8"Y",    u8"Z",    u8"[",    u8"\\",   u8"]",    u8"^",      u8"_",    u8"`",    u8"a",    u8"b",
    u8"c",    u8"d",    u8"e",    u8"f",    u8"g",    u8"h",    u8"i",      u8"j",    u8"k",    u8"l",    u8"m",
    u8"n",    u8"o",    u8"p",    u8"q",    u8"r",    u8"s",    u8"t",      u8"u",    u8"v",    u8"w",    u8"x",
    u8"y",    u8"z",    u8"{",    u8"|",    u8"}",    u8"~",    u8"\u007F", u8"€",    u8"�",    u8"‚",    u8"ƒ",
    u8"„",    u8"…",    u8"†",    u8"‡",    u8"ˆ",    u8"‰",    u8"Š",      u8"‹",    u8"Œ",    u8"�",    u8"Ž",
    u8"�",    u8"�",    u8"‘",    u8"’",    u8"“",    u8"”",    u8"•",      u8"–",    u8"—",    u8"˜",    u8"™",
    u8"š",    u8"›",    u8"œ",    u8"�",    u8"ž",    u8"Ÿ",    u8" ",      u8"¡",    u8"¢",    u8"£",    u8"¤",
    u8"¥",    u8"¦",    u8"§",    u8"¨",    u8"©",    u8"ª",    u8"«",      u8"¬",    u8"­",    u8"®",    u8"¯",
    u8"°",    u8"±",    u8"²",    u8"³",    u8"´",    u8"µ",    u8"¶",      u8"·",    u8"¸",    u8"¹",    u8"º",
    u8"»",    u8"¼",    u8"½",    u8"¾",    u8"¿",    u8"À",    u8"Á",      u8"Â",    u8"Ã",    u8"Ä",    u8"Å",
    u8"Æ",    u8"Ç",    u8"È",    u8"É",    u8"Ê",    u8"Ë",    u8"Ì",      u8"Í",    u8"Î",    u8"Ï",    u8"Ð",
    u8"Ñ",    u8"Ò",    u8"Ó",    u8"Ô",    u8"Õ",    u8"Ö",    u8"×",      u8"Ø",    u8"Ù",    u8"Ú",    u8"Û",
    u8"Ü",    u8"Ý",    u8"Þ",    u8"ß",    u8"à",    u8"á",    u8"â",      u8"ã",    u8"ä",    u8"å",    u8"æ",
    u8"ç",    u8"è",    u8"é",    u8"ê",    u8"ë",    u8"ì",    u8"í",      u8"î",    u8"ï",    u8"ð",    u8"ñ",
    u8"ò",    u8"ó",    u8"ô",    u8"õ",    u8"ö",    u8"÷",    u8"ø",      u8"ù",    u8"ú",    u8"û",    u8"ü",
    u8"ý",    u8"þ",    u8"ÿ"};

struct Utf8ToWin1252Entry {
  uint32_t utf8Bytes;  // Little-endian packed bytes
  uint8_t length;
  char win1252Char;
};
inline constexpr array<Utf8ToWin1252Entry, 128> utf8ToWin1252Table = {{
    // 2-byte sequences (0xC2 prefix: U+00A0 - U+00BF)
    {0xA0C2, 2, static_cast<char>(0xA0)},
    {0xA1C2, 2, static_cast<char>(0xA1)},
    {0xA2C2, 2, static_cast<char>(0xA2)},
    {0xA3C2, 2, static_cast<char>(0xA3)},
    {0xA4C2, 2, static_cast<char>(0xA4)},
    {0xA5C2, 2, static_cast<char>(0xA5)},
    {0xA6C2, 2, static_cast<char>(0xA6)},
    {0xA7C2, 2, static_cast<char>(0xA7)},
    {0xA8C2, 2, static_cast<char>(0xA8)},
    {0xA9C2, 2, static_cast<char>(0xA9)},
    {0xAAC2, 2, static_cast<char>(0xAA)},
    {0xABC2, 2, static_cast<char>(0xAB)},
    {0xACC2, 2, static_cast<char>(0xAC)},
    {0xADC2, 2, static_cast<char>(0xAD)},
    {0xAEC2, 2, static_cast<char>(0xAE)},
    {0xAFC2, 2, static_cast<char>(0xAF)},
    {0xB0C2, 2, static_cast<char>(0xB0)},
    {0xB1C2, 2, static_cast<char>(0xB1)},
    {0xB2C2, 2, static_cast<char>(0xB2)},
    {0xB3C2, 2, static_cast<char>(0xB3)},
    {0xB4C2, 2, static_cast<char>(0xB4)},
    {0xB5C2, 2, static_cast<char>(0xB5)},
    {0xB6C2, 2, static_cast<char>(0xB6)},
    {0xB7C2, 2, static_cast<char>(0xB7)},
    {0xB8C2, 2, static_cast<char>(0xB8)},
    {0xB9C2, 2, static_cast<char>(0xB9)},
    {0xBAC2, 2, static_cast<char>(0xBA)},
    {0xBBC2, 2, static_cast<char>(0xBB)},
    {0xBCC2, 2, static_cast<char>(0xBC)},
    {0xBDC2, 2, static_cast<char>(0xBD)},
    {0xBEC2, 2, static_cast<char>(0xBE)},
    {0xBFC2, 2, static_cast<char>(0xBF)},

    // 2-byte sequences (0xC3 prefix: U+00C0 - U+00FF)
    {0x80C3, 2, static_cast<char>(0xC0)},
    {0x81C3, 2, static_cast<char>(0xC1)},
    {0x82C3, 2, static_cast<char>(0xC2)},
    {0x83C3, 2, static_cast<char>(0xC3)},
    {0x84C3, 2, static_cast<char>(0xC4)},
    {0x85C3, 2, static_cast<char>(0xC5)},
    {0x86C3, 2, static_cast<char>(0xC6)},
    {0x87C3, 2, static_cast<char>(0xC7)},
    {0x88C3, 2, static_cast<char>(0xC8)},
    {0x89C3, 2, static_cast<char>(0xC9)},
    {0x8AC3, 2, static_cast<char>(0xCA)},
    {0x8BC3, 2, static_cast<char>(0xCB)},
    {0x8CC3, 2, static_cast<char>(0xCC)},
    {0x8DC3, 2, static_cast<char>(0xCD)},
    {0x8EC3, 2, static_cast<char>(0xCE)},
    {0x8FC3, 2, static_cast<char>(0xCF)},
    {0x90C3, 2, static_cast<char>(0xD0)},
    {0x91C3, 2, static_cast<char>(0xD1)},
    {0x92C3, 2, static_cast<char>(0xD2)},
    {0x93C3, 2, static_cast<char>(0xD3)},
    {0x94C3, 2, static_cast<char>(0xD4)},
    {0x95C3, 2, static_cast<char>(0xD5)},
    {0x96C3, 2, static_cast<char>(0xD6)},
    {0x97C3, 2, static_cast<char>(0xD7)},
    {0x98C3, 2, static_cast<char>(0xD8)},
    {0x99C3, 2, static_cast<char>(0xD9)},
    {0x9AC3, 2, static_cast<char>(0xDA)},
    {0x9BC3, 2, static_cast<char>(0xDB)},
    {0x9CC3, 2, static_cast<char>(0xDC)},
    {0x9DC3, 2, static_cast<char>(0xDD)},
    {0x9EC3, 2, static_cast<char>(0xDE)},
    {0x9FC3, 2, static_cast<char>(0xDF)},
    {0xA0C3, 2, static_cast<char>(0xE0)},
    {0xA1C3, 2, static_cast<char>(0xE1)},
    {0xA2C3, 2, static_cast<char>(0xE2)},
    {0xA3C3, 2, static_cast<char>(0xE3)},
    {0xA4C3, 2, static_cast<char>(0xE4)},
    {0xA5C3, 2, static_cast<char>(0xE5)},
    {0xA6C3, 2, static_cast<char>(0xE6)},
    {0xA7C3, 2, static_cast<char>(0xE7)},
    {0xA8C3, 2, static_cast<char>(0xE8)},
    {0xA9C3, 2, static_cast<char>(0xE9)},
    {0xAAC3, 2, static_cast<char>(0xEA)},
    {0xABC3, 2, static_cast<char>(0xEB)},
    {0xACC3, 2, static_cast<char>(0xEC)},
    {0xADC3, 2, static_cast<char>(0xED)},
    {0xAEC3, 2, static_cast<char>(0xEE)},
    {0xAFC3, 2, static_cast<char>(0xEF)},
    {0xB0C3, 2, static_cast<char>(0xF0)},
    {0xB1C3, 2, static_cast<char>(0xF1)},
    {0xB2C3, 2, static_cast<char>(0xF2)},
    {0xB3C3, 2, static_cast<char>(0xF3)},
    {0xB4C3, 2, static_cast<char>(0xF4)},
    {0xB5C3, 2, static_cast<char>(0xF5)},
    {0xB6C3, 2, static_cast<char>(0xF6)},
    {0xB7C3, 2, static_cast<char>(0xF7)},
    {0xB8C3, 2, static_cast<char>(0xF8)},
    {0xB9C3, 2, static_cast<char>(0xF9)},
    {0xBAC3, 2, static_cast<char>(0xFA)},
    {0xBBC3, 2, static_cast<char>(0xFB)},
    {0xBCC3, 2, static_cast<char>(0xFC)},
    {0xBDC3, 2, static_cast<char>(0xFD)},
    {0xBEC3, 2, static_cast<char>(0xFE)},
    {0xBFC3, 2, static_cast<char>(0xFF)},

    // 2-byte sequences (CP-1252 specific in 0xC5, 0xC6, 0xCB)
    {0xA0C5, 2, static_cast<char>(0x8A)},  // Š (U+0160)
    {0xA1C5, 2, static_cast<char>(0x9A)},  // š (U+0161)
    {0x92C5, 2, static_cast<char>(0x8C)},  // Œ (U+0152)
    {0x93C5, 2, static_cast<char>(0x9C)},  // œ (U+0153)
    {0xBDC5, 2, static_cast<char>(0x8E)},  // Ž (U+017D)
    {0xBEC5, 2, static_cast<char>(0x9E)},  // ž (U+017E)
    {0xB8C5, 2, static_cast<char>(0x9F)},  // Ÿ (U+0178)
    {0x92C6, 2, static_cast<char>(0x83)},  // ƒ (U+0192)
    {0x86CB, 2, static_cast<char>(0x88)},  // ˆ (U+02C6)
    {0x9CCB, 2, static_cast<char>(0x98)},  // ˜ (U+02DC)

    // 3-byte sequences (CP-1252 specific in 0xE2)
    {0x9AE080E2, 3, static_cast<char>(0x82)},  // ‚ (U+201A)
    {0x9EE080E2, 3, static_cast<char>(0x84)},  // „ (U+201E)
    {0xA6E080E2, 3, static_cast<char>(0x85)},  // … (U+2026)
    {0xA0E080E2, 3, static_cast<char>(0x86)},  // † (U+2020)
    {0xA1E080E2, 3, static_cast<char>(0x87)},  // ‡ (U+2021)
    {0xB0E080E2, 3, static_cast<char>(0x89)},  // ‰ (U+2030)
    {0xB9E080E2, 3, static_cast<char>(0x8B)},  // ‹ (U+2039)
    {0xBAE080E2, 3, static_cast<char>(0x9B)},  // › (U+203A)
    {0x98E080E2, 3, static_cast<char>(0x91)},  // ‘ (U+2018)
    {0x99E080E2, 3, static_cast<char>(0x92)},  // ’ (U+2019)
    {0x9CE080E2, 3, static_cast<char>(0x93)},  // “ (U+201C)
    {0x9DE080E2, 3, static_cast<char>(0x94)},  // ” (U+201D)
    {0xA2E080E2, 3, static_cast<char>(0x95)},  // • (U+2022)
    {0x93E080E2, 3, static_cast<char>(0x96)},  // – (U+2013)
    {0x94E080E2, 3, static_cast<char>(0x97)},  // — (U+2014)
    {0xACE082E2, 3, static_cast<char>(0x80)},  // € (U+20AC)
    {0xA2E084E2, 3, static_cast<char>(0x99)}   // ™ (U+2122)
}};

}  // namespace

std::filesystem::path libbsarchpp::win1252ToPath(std::span<const uint8_t> data) {
  while (data.back() == '\0') {
    data = data.first(data.size() - 1);
  }

  u8string str;
  str.reserve(data.size() * 2);
  for (uint8_t c : data) {
    if (c == '\\') {
      c = '/';
    }
    str.append(win1252ToUtf8[c]);
  }
  return str;
}

std::filesystem::path libbsarchpp::win1252ToPath(std::string_view data) {
  return win1252ToPath(std::span(reinterpret_cast<const uint8_t*>(data.data()), data.size()));
}

std::string libbsarchpp::pathToWin1252(const std::filesystem::path& path) {
  string out;
  u8string str = path.u8string();
  out.reserve(str.size());

  size_t i         = 0;
  const size_t len = str.size();

  while (i < len) {
    auto b0 = static_cast<uint8_t>(str[i]);

    // ascii
    if (b0 < 0x80) {
      out.push_back(static_cast<char>(b0));
      ++i;
      continue;
    }

    if (b0 == 0xC2 && i + 1 < len) {
      out.push_back(static_cast<char>(str[i + 1]));
      i += 2;
      continue;
    }

    if (b0 == 0xC3 && i + 1 < len) {
      out.push_back(static_cast<char>(static_cast<uint8_t>(str[i + 1]) + 0x40));
      i += 2;
      continue;
    }

    bool matched = false;
    for (const auto& [utf8Bytes, length, win1252Char] : utf8ToWin1252Table) {
      if (i + length <= len && std::memcmp(&str[i], &utf8Bytes, length) == 0) {
        out.push_back(win1252Char);
        i += length;
        matched = true;
        break;
      }
    }

    if (!matched) {
      throw runtime_error("invalid character");
    }
  }
  return out;
}

size_t libbsarchpp::getWin1252Length(const std::filesystem::path& path) {
#ifdef __unix__
  const auto& str = path.native();
  return std::ranges::count_if(str, [](unsigned char c) {
    return (c & 0xC0u) != 0x80;
  });
#else
  const auto& wstr = path.native();
  return std::ranges::count_if(wstr, [](wchar_t c) {
    return c < 0xDC00 || c > 0xDFFF;
  });
#endif
}
