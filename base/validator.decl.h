#pragma once
#include <base/types.h>
template <NativeInt value>
static __attribute__((always_inline))
constexpr
bool
is_exact_value(NativeInt i);

template <NativeInt lower_bound, NativeInt upper_bound>
static __attribute__((always_inline))
constexpr
bool
is_in_closed_interval(NativeInt i);

static __attribute__((always_inline))
constexpr
bool
is_mix_byte(NativeInt i);

static __attribute__((always_inline))
constexpr
bool
is_register_index(NativeInt i);

static __attribute__((always_inline))
constexpr
bool
is_mix_address(NativeInt i);

static __attribute__((always_inline))
constexpr
bool
is_mix_word(NativeInt i);

static __attribute__((always_inline))
constexpr
bool
is_mix_positive_word(NativeInt i);