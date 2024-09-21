#pragma once
#include <base/math.decl.h>
namespace mix
{

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
constexpr 
std::array<NativeInt, numerical_bytes_in_extended_word + 1> 
pow_lookup_table(NativeByte base)
{
    std::array<NativeInt, numerical_bytes_in_extended_word + 1> lut;
    lut[0] = 1;
    for (size_t i = 1; i < lut.size(); i++)
        lut[i] = lut[i - 1] * base;
    return lut;
}

constexpr
NativeInt
pow(NativeByte base, size_t exponent)
{
    return pow_lookup_table(base)[exponent];
}

template <size_t exponent>
constexpr 
NativeInt
pow(NativeByte base)
{
    static_assert(0 <= exponent && exponent <= 2 * numerical_bytes_in_word);
    return pow(base, exponent);
}

constexpr auto lut = pow_lookup_table(byte_size);
// Every negative MIX integral value must be representable by NativeInt
static_assert(-(lut.back() - 1) >= std::numeric_limits<NativeInt>::min());
// Every positive MIX integral value must be representable by NativeInt
static_assert(lut.back() - 1 <= std::numeric_limits<NativeInt>::max());

}
