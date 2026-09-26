#include "win1252.h"

#include "utils.h"

#include <array>
#include <cstring>
#include <utility>

using namespace std;

namespace {

constexpr char fallbackChar = '?';

inline constexpr array<u8string_view, 256> win1252ToUtf8 = {
    u8string_view(u8"\x00", 1),
    u8"\x01",
    u8"\x02",
    u8"\x03",
    u8"\x04",
    u8"\x05",
    u8"\x06",
    u8"\x07",
    u8"\x08",
    u8"\x09",
    u8"\x0A",
    u8"\x0B",
    u8"\x0C",
    u8"\x0D",
    u8"\x0E",
    u8"\x0F",
    u8"\x10",
    u8"\x11",
    u8"\x12",
    u8"\x13",
    u8"\x14",
    u8"\x15",
    u8"\x16",
    u8"\x17",
    u8"\x18",
    u8"\x19",
    u8"\x1A",
    u8"\x1B",
    u8"\x1C",
    u8"\x1D",
    u8"\x1E",
    u8"\x1F",
    u8" ",
    u8"!",
    u8"\"",
    u8"#",
    u8"$",
    u8"%",
    u8"&",
    u8"'",
    u8"(",
    u8")",
    u8"*",
    u8"+",
    u8",",
    u8"-",
    u8".",
    u8"/",
    u8"0",
    u8"1",
    u8"2",
    u8"3",
    u8"4",
    u8"5",
    u8"6",
    u8"7",
    u8"8",
    u8"9",
    u8":",
    u8";",
    u8"<",
    u8"=",
    u8">",
    u8"?",
    u8"@",
    u8"A",
    u8"B",
    u8"C",
    u8"D",
    u8"E",
    u8"F",
    u8"G",
    u8"H",
    u8"I",
    u8"J",
    u8"K",
    u8"L",
    u8"M",
    u8"N",
    u8"O",
    u8"P",
    u8"Q",
    u8"R",
    u8"S",
    u8"T",
    u8"U",
    u8"V",
    u8"W",
    u8"X",
    u8"Y",
    u8"Z",
    u8"[",
    u8"\\",
    u8"]",
    u8"^",
    u8"_",
    u8"`",
    u8"a",
    u8"b",
    u8"c",
    u8"d",
    u8"e",
    u8"f",
    u8"g",
    u8"h",
    u8"i",
    u8"j",
    u8"k",
    u8"l",
    u8"m",
    u8"n",
    u8"o",
    u8"p",
    u8"q",
    u8"r",
    u8"s",
    u8"t",
    u8"u",
    u8"v",
    u8"w",
    u8"x",
    u8"y",
    u8"z",
    u8"{",
    u8"|",
    u8"}",
    u8"~",
    u8"\x7F",
    u8"\xE2\x82\xAC",  // 0x80: € (U+20AC Euro Sign)
    u8"\xEF\xBF\xBD",  // 0x81: undefined (U+FFFD Replacement Character)
    u8"\xE2\x80\x9A",  // 0x82: ‚ (U+201A Single Low-9 Quotation Mark)
    u8"\xC6\x92",      // 0x83: ƒ (U+0192 Latin Small Letter F With Hook)
    u8"\xE2\x80\x9E",  // 0x84: „ (U+201E Double Low-9 Quotation Mark)
    u8"\xE2\x80\xA6",  // 0x85: … (U+2026 Horizontal Ellipsis)
    u8"\xE2\x80\xA0",  // 0x86: † (U+2020 Dagger)
    u8"\xE2\x80\xA1",  // 0x87: ‡ (U+2021 Double Dagger)
    u8"\xCB\x86",      // 0x88: ˆ (U+02C6 Modifier Letter Circumflex Accent)
    u8"\xE2\x80\xB0",  // 0x89: ‰ (U+2030 Per Mille Sign)
    u8"\xC5\xA0",      // 0x8A: Š (U+0160 Latin Capital Letter S With Caron)
    u8"\xE2\x80\xB9",  // 0x8B: ‹ (U+2039 Single Left-Pointing Angle Quotation Mark)
    u8"\xC5\x92",      // 0x8C: Œ (U+0152 Latin Capital Ligature OE)
    u8"\xEF\xBF\xBD",  // 0x8D: undefined (U+FFFD Replacement Character)
    u8"\xC5\xBD",      // 0x8E: Ž (U+017D Latin Capital Letter Z With Caron)
    u8"\xEF\xBF\xBD",  // 0x8F: undefined (U+FFFD Replacement Character)
    u8"\xEF\xBF\xBD",  // 0x90: undefined (U+FFFD Replacement Character)
    u8"\xE2\x80\x98",  // 0x91: ‘ (U+2018 Left Single Quotation Mark)
    u8"\xE2\x80\x99",  // 0x92: ’ (U+2019 Right Single Quotation Mark)
    u8"\xE2\x80\x9C",  // 0x93: “ (U+201C Left Double Quotation Mark)
    u8"\xE2\x80\x9D",  // 0x94: ” (U+201D Right Double Quotation Mark)
    u8"\xE2\x80\xA2",  // 0x95: • (U+2022 Bullet)
    u8"\xE2\x80\x93",  // 0x96: – (U+2013 En Dash)
    u8"\xE2\x80\x94",  // 0x97: — (U+2014 Em Dash)
    u8"\xCB\x9C",      // 0x98: ˜ (U+02DC Small Tilde)
    u8"\xE2\x84\xA2",  // 0x99: ™ (U+2122 Trade Mark Sign)
    u8"\xC5\xA1",      // 0x9A: š (U+0161 Latin Small Letter S With Caron)
    u8"\xE2\x80\xBA",  // 0x9B: › (U+203A Single Right-Pointing Angle Quotation Mark)
    u8"\xC5\x93",      // 0x9C: œ (U+0153 Latin Small Ligature OE)
    u8"\xEF\xBF\xBD",  // 0x9D: undefined (U+FFFD Replacement Character)
    u8"\xC5\xBE",      // 0x9E: ž (U+017E Latin Small Letter Z With Caron)
    u8"\xC5\xB8",      // 0x9F: Ÿ (U+0178 Latin Capital Letter Y With Diaeresis)
    u8"\xC2\xA0",      // 0xA0: Non-Breaking Space
    u8"\xC2\xA1",      // 0xA1: ¡
    u8"\xC2\xA2",      // 0xA2: ¢
    u8"\xC2\xA3",      // 0xA3: £
    u8"\xC2\xA4",      // 0xA4: ¤
    u8"\xC2\xA5",      // 0xA5: ¥
    u8"\xC2\xA6",      // 0xA6: ¦
    u8"\xC2\xA7",      // 0xA7: §
    u8"\xC2\xA8",      // 0xA8: ¨
    u8"\xC2\xA9",      // 0xA9: ©
    u8"\xC2\xAA",      // 0xAA: ª
    u8"\xC2\xAB",      // 0xAB: «
    u8"\xC2\xAC",      // 0xAC: ¬
    u8"\xC2\xAD",      // 0xAD: Soft Hyphen
    u8"\xC2\xAE",      // 0xAE: ®
    u8"\xC2\xAF",      // 0xAF: ¯
    u8"\xC2\xB0",      // 0xB0: °
    u8"\xC2\xB1",      // 0xB1: ±
    u8"\xC2\xB2",      // 0xB2: ²
    u8"\xC2\xB3",      // 0xB3: ³
    u8"\xC2\xB4",      // 0xB4: ´
    u8"\xC2\xB5",      // 0xB5: µ
    u8"\xC2\xB6",      // 0xB6: ¶
    u8"\xC2\xB7",      // 0xB7: ·
    u8"\xC2\xB8",      // 0xB8: ¸
    u8"\xC2\xB9",      // 0xB9: ¹
    u8"\xC2\xBA",      // 0xBA: º
    u8"\xC2\xBB",      // 0xBB: »
    u8"\xC2\xBC",      // 0xBC: ¼
    u8"\xC2\xBD",      // 0xBD: ½
    u8"\xC2\xBE",      // 0xBE: ¾
    u8"\xC2\xBF",      // 0xBF: ¿
    u8"\xC3\x80",      // 0xC0: À
    u8"\xC3\x81",      // 0xC1: Á
    u8"\xC3\x82",      // 0xC2: Â
    u8"\xC3\x83",      // 0xC3: Ã
    u8"\xC3\x84",      // 0xC4: Ä
    u8"\xC3\x85",      // 0xC5: Å
    u8"\xC3\x86",      // 0xC6: Æ
    u8"\xC3\x87",      // 0xC7: Ç
    u8"\xC3\x88",      // 0xC8: È
    u8"\xC3\x89",      // 0xC9: É
    u8"\xC3\x8A",      // 0xCA: Ê
    u8"\xC3\x8B",      // 0xCB: Ë
    u8"\xC3\x8C",      // 0xCC: Ì
    u8"\xC3\x8D",      // 0xCD: Í
    u8"\xC3\x8E",      // 0xCE: Î
    u8"\xC3\x8F",      // 0xCF: Ï
    u8"\xC3\x90",      // 0xD0: Ð
    u8"\xC3\x91",      // 0xD1: Ñ
    u8"\xC3\x92",      // 0xD2: Ò
    u8"\xC3\x93",      // 0xD3: Ó
    u8"\xC3\x94",      // 0xD4: Ô
    u8"\xC3\x95",      // 0xD5: Õ
    u8"\xC3\x96",      // 0xD6: Ö
    u8"\xC3\x97",      // 0xD7: ×
    u8"\xC3\x98",      // 0xD8: Ø
    u8"\xC3\x99",      // 0xD9: Ù
    u8"\xC3\x9A",      // 0xDA: Ú
    u8"\xC3\x9B",      // 0xDB: Û
    u8"\xC3\x9C",      // 0xDC: Ü
    u8"\xC3\x9D",      // 0xDD: Ý
    u8"\xC3\x9E",      // 0xDE: Þ
    u8"\xC3\x9F",      // 0xDF: ß
    u8"\xC3\xA0",      // 0xE0: à
    u8"\xC3\xA1",      // 0xE1: á
    u8"\xC3\xA2",      // 0xE2: â
    u8"\xC3\xA3",      // 0xE3: ã
    u8"\xC3\xA4",      // 0xE4: ä
    u8"\xC3\xA5",      // 0xE5: å
    u8"\xC3\xA6",      // 0xE6: æ
    u8"\xC3\xA7",      // 0xE7: ç
    u8"\xC3\xA8",      // 0xE8: è
    u8"\xC3\xA9",      // 0xE9: é
    u8"\xC3\xAA",      // 0xEA: ê
    u8"\xC3\xAB",      // 0xEB: ë
    u8"\xC3\xAC",      // 0xEC: ì
    u8"\xC3\xAD",      // 0xED: í
    u8"\xC3\xAE",      // 0xEE: î
    u8"\xC3\xAF",      // 0xEF: ï
    u8"\xC3\xB0",      // 0xF0: ð
    u8"\xC3\xB1",      // 0xF1: ñ
    u8"\xC3\xB2",      // 0xF2: ò
    u8"\xC3\xB3",      // 0xF3: ó
    u8"\xC3\xB4",      // 0xF4:  ô
    u8"\xC3\xB5",      // 0xF5: õ
    u8"\xC3\xB6",      // 0xF6: ö
    u8"\xC3\xB7",      // 0xF7: ÷
    u8"\xC3\xB8",      // 0xF8: ø
    u8"\xC3\xB9",      // 0xF9: ù
    u8"\xC3\xBA",      // 0xFA: ú
    u8"\xC3\xBB",      // 0xFB: û
    u8"\xC3\xBC",      // 0xFC: ü
    u8"\xC3\xBD",      // 0xFD: ý
    u8"\xC3\xBE",      // 0xFE: þ
    u8"\xC3\xFF"       // 0xFF: ÿ
};

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

std::string fromUtf8(const std::u8string_view str) {
  string out;
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
      out.push_back(fallbackChar);
      // Skip the current multi-byte sequence
      if ((b0 & 0xE0u) == 0xC0) {
        i += 2;
      } else if ((b0 & 0xF0u) == 0xE0) {
        i += 3;
      } else if ((b0 & 0xF8u) == 0xF0) {
        i += 4;
      } else {
        ++i;
      }
    }
  }
  return out;
}

}  // namespace

Win1252string::Win1252string(std::string rawData) : m_data(std::move(rawData)) {
  normalizeSeparators();
}
Win1252string::Win1252string(const std::vector<uint8_t>& rawData)
    : m_data(reinterpret_cast<const char*>(rawData.data()), rawData.size()) {
  normalizeSeparators();
}
Win1252string::Win1252string(std::u8string_view u8Str) : m_data(fromUtf8(u8Str)) {
  normalizeSeparators();
}

Win1252string::Win1252string(const std::filesystem::path& path) : m_data(fromUtf8(path.generic_u8string())) {
  normalizeSeparators();
}

Win1252string& Win1252string::operator=(std::string str) {
  m_data = std::move(str);
  return *this;
}

std::u8string Win1252string::toU8String() const {
  u8string utf8;
  utf8.reserve(m_data.size() * 2);
  for (const uint8_t c : m_data) {
    utf8.append(win1252ToUtf8[c]);
  }

  return utf8;
}

std::string Win1252string::toUtf8() const {
  const u8string str = toU8String();
  return {reinterpret_cast<const char*>(str.data()), str.size()};
}

std::string_view Win1252string::parentPath() const {
  return libbsarchpp::getParentPath(m_data);
}

void Win1252string::normalizePath() {
  libbsarchpp::normalizePath(m_data);
}

std::string_view Win1252string::filename() const {
  const size_t lastSlash = m_data.find_last_of('/');
  if (lastSlash == string_view::npos) {
    return m_data;
  }
  return {m_data.data() + lastSlash + 1};
}

std::string_view Win1252string::extension() const {
  const size_t lastDot = m_data.find_last_of('.');
  if (lastDot == string::npos) {
    return m_data;
  }

  return {m_data.data() + lastDot};
}

std::string_view Win1252string::stem() const {
  const auto fileName  = filename();
  const size_t lastDot = fileName.find_last_of('.');
  if (lastDot == string::npos) {
    return fileName;
  }

  return {fileName.data(), lastDot};
}
