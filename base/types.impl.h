#pragma once
#include <base/types.decl.h>
namespace mix
{

constexpr 
Sign
sign(NativeInt value)
{
    if (value == 0)
        throw std::runtime_error("Unknown sign");
    return value > 0 ? s_plus : s_minus;
}

}
