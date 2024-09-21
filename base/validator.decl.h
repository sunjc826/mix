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

}
