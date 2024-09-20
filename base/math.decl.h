#pragma once
#include <base/types.h>

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
static __attribute__((always_inline))
constexpr
std::array<NativeInt, numerical_bytes_in_extended_word + 1> 
pow_lookup_table(NativeByte base);

static __attribute__((always_inline))
constexpr
NativeInt
pow(NativeByte base, size_t exponent);

template <size_t exponent>
static __attribute__((always_inline))
constexpr
NativeInt
pow(NativeByte base);