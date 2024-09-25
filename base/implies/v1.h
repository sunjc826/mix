#pragma once
#include "check.h"
#include <base/types.h>
#include <base/validation/validator.h>

namespace mix
{

// v1 only supports tree-like graphs of implications
// if we wish to avoid ambiguous implicit conversions.
// For example, if we specify
// Implies<P, Q1>, Implies<P, Q2>, Implies<Q1, Q3>, Implies<Q2, Q3>
// then the implicit conversion from 
// ValidatedInt<validator = P> to ValidatedInt<validator = Q3>
// will not succeed.

template <Validator P, Validator Q>
struct Implies
{
    static constexpr bool value = false;
};

template <Validator P, Validator Q>
constexpr bool Implies_v = Implies<P, Q>::value;

#define IMPLIES(...) \
template <> \
struct Implies<__VA_ARGS__> \
{ \
    static constexpr bool value = true; \
};

IMPLIES(is_exact_value<0>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<1>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<2>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<3>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<4>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<5>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<6>, is_in_closed_interval<0, 6>)

IMPLIES(is_in_closed_interval<0, 6>, is_register_index)

IMPLIES(is_register_index, is_mix_byte)

IMPLIES(is_mix_byte, is_mix_address)

IMPLIES(is_mix_address, is_mix_positive_word)

IMPLIES(is_mix_positive_word, is_mix_word)

#undef IMPLIES

}
