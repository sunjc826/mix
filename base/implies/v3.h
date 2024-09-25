#pragma once
#include "base/validation/validator.impl.h"
#include <base/types.h>
#include <base/validation/validator.h>
#include <type_traits>

namespace mix
{

template <typename P>
struct DirectImplications
{
    using type = std::tuple<>;
};

template <typename ValidatorT1, typename ValidatorT2>
struct DirectImplications<And<ValidatorT1, ValidatorT2>>
{
    using type = std::tuple<ValidatorT1, ValidatorT2>;
}; 

template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
#define IMPLIES(P, ...) \
template <> \
struct DirectImplications<argument_type<void(P)>::type> \
{ \
    using type = std::tuple<__VA_ARGS__>; \
};

IMPLIES(IsExactValue<0>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<1>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<2>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<3>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<4>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<5>, IsInClosedInterval<0, 6>)

IMPLIES(IsExactValue<6>, IsInClosedInterval<0, 6>)

IMPLIES((IsInClosedInterval<0, 6>), IsRegisterIndex)

IMPLIES(IsRegisterIndex, IsMixByte)

IMPLIES((IsInClosedInterval<0, byte_size - 1>), IsMixByte)

IMPLIES(IsMixByte, IsMixAddress)

IMPLIES((IsInClosedInterval<0, 3999>), IsMixAddress)

IMPLIES(IsMixAddress, IsMixPositiveWord)

IMPLIES(IsMixPositiveWord, IsMixWord, IsPositive)

IMPLIES(IsPositive, IsNonNegative)

#undef IMPLIES

// https://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
template <typename T, typename Tuple>
struct has_type;

template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

template <typename P, typename Q>
struct NonTransitiveImplies
{
    static constexpr bool value = std::is_same_v<P, Q> || has_type<Q, typename DirectImplications<P>::type>::value;
};

template <typename P, typename Q>
constexpr bool Implies_v = NonTransitiveImplies<P, Q>::value;

template <typename ValidatorT1, typename ValidatorT2>
struct NonTransitiveImplies<And<ValidatorT1, ValidatorT2>, And<ValidatorT2, ValidatorT1>>
{
    static constexpr bool value = true;
};

template <typename ValidatorT1, typename ValidatorT2, typename ValidatorT3>
struct NonTransitiveImplies<ValidatorT1, And<ValidatorT2, ValidatorT3>>
{
    static constexpr bool value = Implies_v<ValidatorT1, ValidatorT2> && Implies_v<ValidatorT1, ValidatorT3>;
};

template <NativeInt literal, NativeInt low, NativeInt high>
struct NonTransitiveImplies<IsExactValue<literal>, IsInClosedInterval<low, high>>
{
    static constexpr bool value = low <= literal && literal <= high;
};

template <NativeInt low1, NativeInt high1,  NativeInt low2, NativeInt high2>
struct NonTransitiveImplies<IsInClosedInterval<low1, high1>, IsInClosedInterval<low2, high2>>
{
    static constexpr bool value = low2 <= low1 && high1 <= high2;
};


template <typename from, typename to>
consteval bool implies();
namespace details
{
template <typename from, typename to, size_t ... Is>
consteval bool implies_helper(std::integer_sequence<size_t, Is...> const &)
{
   return (implies<std::tuple_element_t<Is, typename DirectImplications<from>::type>, to>() or ...);
}
}

template <typename P, typename Q>
consteval bool implies()
{
    if constexpr (Implies_v<P, Q>)
        return true;
    constexpr size_t sz = std::tuple_size_v<typename DirectImplications<P>::type>;
    if constexpr (sz == 0)
        return false;
    return details::implies_helper<P, Q>(std::make_integer_sequence<size_t, sz>());
}

template <typename P, typename Q, typename FirstHintT, typename ...HintsT>
consteval bool implies()
{
    if constexpr (implies<P, FirstHintT>() && implies<FirstHintT, Q, HintsT...>())
        return true;
    
    return implies<P, Q>();
}

static_assert(!implies<IsExactValue<0>, IsInClosedInterval<31, 42>>());
static_assert(implies<IsExactValue<34>, IsInClosedInterval<31, 42>>());
static_assert(implies<
    IsInClosedInterval<5, 8>,
    And<IsInClosedInterval<1, 10>, IsInClosedInterval<2, 11>>
>());
static_assert(implies<
    IsExactValue<2>,
    And<IsInClosedInterval<1, 10>, IsInClosedInterval<2, 11>>
>());

static_assert(implies<IsExactValue<2>, IsMixPositiveWord>());
// Unfortunately this won't work
// It is not possible to get transitivity when Implies_v is used. 
// static_assert(implies<IsExactValue<7>, IsMixPositiveWord>());

// However, with some deduction hints, we can do it!
static_assert(implies<
    IsExactValue<7>, 
    IsMixPositiveWord,
    IsInClosedInterval<0, 3999>
>());

}

