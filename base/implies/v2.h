#pragma once
#include "check.h"
#include <base/types.h>
#include <base/validation/validator.h>
#include <type_traits>

namespace mix
{
// v2 allows arbitrary DAGS
// No cycles though.

template <IntValidator P>
struct DirectImplications
{
    static constexpr std::array<IntValidator, 0> implications = {};
};

#define IMPLIES(P, ...) \
template <> \
struct DirectImplications<P> \
{ \
    static constexpr std::array implications = {__VA_ARGS__}; \
};


IMPLIES(is_exact_value<0>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<1>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<2>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<3>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<4>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<5>, is_in_closed_interval<0, 6>)

IMPLIES(is_exact_value<6>, is_in_closed_interval<0, 6>)

IMPLIES((is_in_closed_interval<0, 6>), is_register_index)

IMPLIES(is_register_index, is_mix_byte)

IMPLIES(is_mix_byte, is_mix_address)

IMPLIES(is_mix_address, is_mix_positive_word)

IMPLIES(is_mix_positive_word, is_mix_word)

#undef IMPLIES

template <IntValidator from, IntValidator to>
consteval bool implies();
namespace details
{
template <IntValidator from, IntValidator to, size_t ... Is>
consteval bool implies_helper(std::integer_sequence<size_t, Is...> const &)
{
   return (implies<DirectImplications<from>::implications[Is], to>() or ...);
}
}

template <IntValidator from, IntValidator to>
consteval bool implies()
{
    if constexpr(from == to)
        return true;
    constexpr size_t sz = DirectImplications<from>::implications.size();
    if constexpr (sz == 0)
        return false;
    return details::implies_helper<from, to>(std::make_integer_sequence<size_t, sz>());
}



}
