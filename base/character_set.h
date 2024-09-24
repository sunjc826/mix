#pragma once
#include <base/types.h>
#include <base/function.h>

#include <array>
#include <string_view>
namespace mix
{
struct Char
{
    std::string_view utf8_value;
    constexpr Char(char const *utf8_value) : utf8_value(utf8_value) {}
    constexpr Char(std::string_view utf8_value) : utf8_value(utf8_value) {}
    constexpr bool operator==(Char const &other) const = default;
};

constexpr std::array<Char, 56> character_set
{
    " ",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "Δ",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "Σ",
    "Π",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ".",
    ",",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "=",
    "$",
    "<",
    ">",
    "@",
    ";",
    ":",
    "'",
};

inline constexpr
bool 
isdigit(NativeByte ch)
{
    return 30 <= ch && ch <= 39;
}

inline constexpr
bool
isalpha(NativeByte ch)
{
    return 1 <= ch && ch <= 29;
}

inline constexpr
bool
isspace(NativeByte ch)
{
    return ch == 0;
}

inline constexpr
bool
isalnum(NativeByte ch)
{
    return 1 <= ch && ch <= 39;
}

inline constexpr
bool
is_mix_char(NativeInt ch)
{
    return ch <= 55;
}

using IsMixChar = ValidatorToFunctor<is_mix_char>;

}
