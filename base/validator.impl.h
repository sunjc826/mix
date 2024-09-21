#pragma once
#include <base/validator.decl.h>
#include <base/math.h>
namespace mix
{

template <NativeInt value>
constexpr
bool
is_exact_value(NativeInt i)
{
    return value == i;
}

template <NativeInt lower_bound, NativeInt upper_bound>
constexpr
bool
is_in_closed_interval(NativeInt i)
{
    static_assert(lower_bound <= upper_bound);
    return lower_bound <= i && i < upper_bound;
}

constexpr
bool 
is_mix_byte(NativeInt i)
{
    return 0 <= i && i < byte_size;
}

constexpr
bool 
is_register_index(NativeInt i)
{
    return 0 <= i && i <= 6;
}

constexpr
bool 
is_mix_address(NativeInt i)
{
    return 0 <= i && i < main_memory_size;
}

constexpr
bool 
is_mix_word(NativeInt i)
{
    return -lut[numerical_bytes_in_word] < i && i < lut[numerical_bytes_in_word];
}

constexpr
bool
is_mix_positive_word(NativeInt i)
{
    return 0 < i && i < lut[numerical_bytes_in_word];
}

}
