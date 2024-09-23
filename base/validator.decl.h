#pragma once
#include <base/types.h>
namespace mix
{
    
template <NativeInt value>
[[gnu::always_inline]]
static constexpr
bool
is_exact_value(NativeInt i);


template <NativeInt lower_bound, NativeInt upper_bound>
[[gnu::always_inline]]
static constexpr
bool
is_in_closed_interval(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_mix_byte(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_register_index(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_mix_address(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_mix_word(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_mix_positive_word(NativeInt i);

template <NativeInt literal>
struct IsExactValue
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_exact_value<literal>(i);
    }
};

template <NativeInt low, NativeInt high>
struct IsInClosedInterval
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_in_closed_interval<low, high>(i);
    }
};

struct IsMixByte
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_mix_byte(i);
    }
};

struct IsRegisterIndex
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_register_index(i);
    }
};

struct IsMixAddress 
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_mix_address(i);
    }
};

struct IsMixWord
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_mix_word(i);
    }
};

struct IsMixPositiveWord
{
    static constexpr bool operator()(NativeInt i)
    {
        return is_mix_positive_word(i);
    }
};

}
