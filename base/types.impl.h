#pragma once
#include <base/types.decl.h>

// TODO: change to use Result datatype instead of throwing exceptions
void 
check_address_bounds(NativeInt value)
{
    if (value < 0)
        throw std::runtime_error("Negative address");
    else if (value >= main_memory_size)
        throw std::runtime_error("Overflowing address");
}

constexpr 
NativeInt 
native_sign(Sign sign)
{
    return sign == s_plus ? 1 : -1; 
}

constexpr 
Sign
sign(NativeInt value)
{
    if (value == 0)
        throw std::runtime_error("Unknown sign");
    return value > 0 ? s_plus : s_minus;
}

template <size_t size>
ByteConversionResult<size> as_bytes(NativeInt value)
{
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

    ByteConversionResult<size> result;
    for (size_t s = size; s --> 1;)
    {
        result.bytes[s].byte = value % byte_size;
        value /= byte_size;
    }

    result.bytes[0].sign = sign;
    
    result.overflow = value > 0;

    return result;
}
