#pragma once
#include <base/types.h>
namespace mix
{
// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
[[gnu::always_inline]] 
static constexpr
std::array<NativeInt, numerical_bytes_in_extended_word + 1> 
pow_lookup_table(NativeByte base);

[[gnu::always_inline]] 
static constexpr
NativeInt
pow(NativeByte base, size_t exponent);

template <size_t exponent>
[[gnu::always_inline]]
static constexpr
NativeInt
pow(NativeByte base);

}