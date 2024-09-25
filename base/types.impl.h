#pragma once
#include <base/types.decl.h>
namespace mix
{

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

}
