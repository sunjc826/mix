#pragma once
#include <base/types.h>
#include <base/validation/validator.h>
#include <type_traits>

namespace mix
{
// v2 allows arbitrary DAGS
// No cycles though.

template <typename P, typename Q>
struct Implies
{
    static constexpr bool value = false;
};
template <typename P, typename Q>
constexpr bool Implies_v = Implies<P, Q>::value;

template <NativeInt literal, NativeInt low, NativeInt high>
struct Implies<IsExactValue<literal>, IsInClosedInterval<low, high>>
{
    static constexpr bool value = low <= literal && literal <= high;
};

template <typename P>
struct DirectImplications
{
    using type = std::tuple<>;
};

template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
#define IMPLIES(P, ...) \
template <> \
struct DirectImplications<argument_type<void(P)>::type> \
{ \
    using type = std::tuple<__VA_ARGS__>; \
};

IMPLIES((IsInClosedInterval<0, 6>), IsRegisterIndex)

IMPLIES(IsRegisterIndex, IsMixByte)

IMPLIES(IsMixByte, IsMixAddress)

IMPLIES(IsMixAddress, IsMixPositiveWord)

IMPLIES(IsMixPositiveWord, IsMixWord)

#undef IMPLIES

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

template <typename from, typename to>
consteval bool implies()
{
    if constexpr(std::is_same_v<from, to>)
        return true;
    if constexpr (Implies_v<from, to>)
        return true;
    constexpr size_t sz = std::tuple_size_v<typename DirectImplications<from>::type>;
    if constexpr (sz == 0)
        return false;
    return details::implies_helper<from, to>(std::make_integer_sequence<size_t, sz>());
}

static_assert(!implies<IsExactValue<0>, IsInClosedInterval<31, 42>>());
static_assert(implies<IsExactValue<34>, IsInClosedInterval<31, 42>>());

}

