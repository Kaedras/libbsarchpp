#include "utils.h"

#include "types.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <stdexcept>

using namespace std;

namespace libbsarchpp {

uint32_t MagicToInt(const Magic4 value) noexcept
{
    uint32_t result;
    memcpy(&result, value.data(), 4);
    return result;
}

Magic4 IntToMagic(const uint32_t value) noexcept
{
    Magic4 result;
    memcpy(result.data(), &value, 4);
    return result;
}

Magic4 StringToMagic(const std::string &str) noexcept
{
    Magic4 result = {0, 0, 0, 0};

    for (size_t i = 0; i < 4; i++)
    {
        if (str.length() > i)
        {
            result[i] = str[i];
        }
        else
        {
            break;
        }
    }
    return result;
}

std::string MagicToString(const Magic4 &magic) noexcept
{
    std::string str;
    for (const char c : magic)
    {
        str += c;
    }
    return str;
}

std::string MagicToString(const uint32_t &value) noexcept
{
    std::string str;
    str += static_cast<char>(value & 0xFF);
    str += static_cast<char>(value >> 8 & 0xFF);
    str += static_cast<char>(value >> 16 & 0xFF);
    str += static_cast<char>(value >> 24 & 0xFF);

    return str;
}

std::string ToLower(std::string str) noexcept
{
    ranges::transform(str, str.begin(), ::tolower);
    return str;
}

std::string ToLower(const std::filesystem::path &str) noexcept
{
    return ToLower(str.string());
}

void ToLowerInline(std::string &str) noexcept
{
    std::ranges::transform(str, str.begin(), ::tolower);
}

void normalizePath(std::string &str) noexcept
{
    // replace '/' with '\\'
    std::ranges::replace(str, '/', '\\');
    // change string to lower characters
    std::ranges::transform(str, str.begin(), ::tolower);
}

void changeSlashesToBackslashes(std::u16string &str) noexcept
{
    for (auto &c : str)
    {
        if (c == '/')
        {
            c = '\\';
        }
    }
}

bool comparePaths(const std::filesystem::path &lhs, const std::filesystem::path &rhs) noexcept(false)
{
    const string lhsStr = lhs.generic_string();
    const string rhsStr = rhs.generic_string();

    // approximate sorting order:
    // '/' == '\' < ' '
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

    for (unsigned long i = 0; i < lhsStr.length() && i < rhsStr.length(); i++)
    {
        const unsigned char &l = lhsStr[i];
        const unsigned char &r = rhsStr[i];

        // continue if both chars are identical
        if (tolower(l) == tolower(r))
        {
            continue;
        }

        // if one char is '/' or '\\'
        if (l == '/' || l == '\\' || r == '/' || r == '\\')
        {
            return l == '/' || l == '\\';
        }
        // if one char is ' '
        if (l == ' ' || r == ' ')
        {
            return l == ' ';
        }
        // if one char is '!'
        if (l == '!' || r == '!')
        {
            return l == '!';
        }
        // if one char is '#'
        if (l == '#' || r == '#')
        {
            return l == '#';
        }
        // if one char is '$'
        if (l == '$' || r == '$')
        {
            return l == '$';
        }
        // if one char is '%'
        if (l == '%' || r == '%')
        {
            return l == '%';
        }
        // if one char is '&'
        if (l == '&' || r == '&')
        {
            return l == '&';
        }
        // if one char is '\''
        if (l == '\'' || r == '\'')
        {
            return l == '\'';
        }
        // if one char is '('
        if (l == '(' || r == '(')
        {
            return l == '(';
        }
        // if one char is ')'
        if (l == ')' || r == ')')
        {
            return l == ')';
        }
        // if one char is '+'
        if (l == '+' || r == '+')
        {
            return l == '+';
        }
        // if one char is ','
        if (l == ',' || r == ',')
        {
            return l == ',';
        }
        // if one char is '-'
        if (l == '-' || r == '-')
        {
            return l == '-';
        }
        // if one char is '.'
        if (l == '.' || r == '.')
        {
            return l == '.';
        }
        // if only one char is a digit
        if (static_cast<bool>(isdigit(l)) ^ static_cast<bool>(isdigit(r)))
        {
            return isdigit(l) != 0;
        }
        // if both chars are digits
        if (static_cast<bool>(isdigit(l)) && static_cast<bool>(isdigit(r)))
        {
            return l < r;
        }
        // if one char is ';'
        if (l == ';' || r == ';')
        {
            return l == ';';
        }
        // if one char is '='
        if (l == '=' || r == '=')
        {
            return l == '=';
        }
        // if one char is '@'
        if (l == '@' || r == '@')
        {
            return l == '@';
        }
        // if only one char is alphabetical
        if (static_cast<bool>(isalpha(l)) ^ static_cast<bool>(isalpha(r)))
        {
            return isalpha(l) != 0;
        }
        // if both chars are alphabetical
        if (static_cast<bool>(isalpha(l)) && static_cast<bool>(isalpha(r)))
        {
            return tolower(l) < tolower(r);
        }
        // if both chars are not ascii
        if (!static_cast<bool>(isascii(l)) && !static_cast<bool>(isascii(r)))
        {
            return l < r;
        }
        // if one char is '['
        if (l == '[' || r == '[')
        {
            return l == '[';
        }
        // if one char is ']'
        if (l == ']' || r == ']')
        {
            return l == ']';
        }
        // if one char is '^'
        if (l == '^' || r == '^')
        {
            return l == '^';
        }
        // if one char is '_'
        if (l == '_' || r == '_')
        {
            return l == '_';
        }
        // if one char is '`'
        if (l == '`' || r == '`')
        {
            return l == '`';
        }
        // if one char is '{'
        if (l == '{' || r == '{')
        {
            return l == '{';
        }
        // if one char is '}'
        if (l == '}' || r == '}')
        {
            return l == '}';
        }
        // if one char is '~'
        if (l == '~' || r == '~')
        {
            return l == '~';
        }

        // unexpected character, this should not happen
        throw runtime_error(format("{}: internal error, characters are {} and {}", __FUNCTION__, l, r));
    }

    return lhsStr.length() < rhsStr.length();
}

} // namespace libbsarchpp