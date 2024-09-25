#pragma once
#include <base/types.h>
#include <base/function.h>
namespace mix
{

template <NativeInt value>
[[gnu::always_inline]]
static constexpr
bool
is_exact_value(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_positive(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_negative(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_nonnegative(NativeInt i);

[[gnu::always_inline]]
static constexpr
bool
is_nonpositive(NativeInt i);

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

template <typename ValidatorT1, typename ValidatorT2>
struct And
{
    static ValidatorT1 v1;
    static ValidatorT2 v2;
    constexpr
    bool operator()(NativeInt i) const
    {
        return v1(i) && v2(i);
    }
};

}
