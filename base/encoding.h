#pragma once
#include <base/character_set.h>
#include <base/result.h>
#include <string_view>
namespace mix
{

// Converts utf8 string to mix string 
class Decoder
{
    Result<std::basic_string_view<NativeByte>> transform(std::basic_string_view<char> utf8_str)
    {

    }
};

// Converts mix string to utf8 string
class Encoder
{
    Result<std::basic_string_view<char>> transform(std::basic_string_view<NativeByte> mix_str)
    {

    }
};

}
