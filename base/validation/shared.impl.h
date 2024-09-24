#pragma once
#include <base/validation/shared.decl.h>

namespace mix
{
    
std::array<Byte, bytes_in_word> 
as_bytes(ValidatedWord word)
{
    NativeInt value = word;
    Sign sign;
    if (value < 0)
    {
        sign = s_minus;
        value = -value;
    }
    else
    {
        sign = s_plus;
    }

    std::array<Byte, bytes_in_word> result;
    for (size_t s = result.size(); s --> 1;)
    {
        result[s].byte = value % byte_size;
        value /= byte_size;
    }

    result[0].sign = sign;

    return result;
}

}
