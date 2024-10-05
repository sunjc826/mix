#pragma once
#include "check.h"
#include <base/types.h>
#include <base/type_sequence.h>
#include <base/validation/validator.h>
#include <type_traits>
#include <utility>

namespace mix
{

// https://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
template <typename T, typename Tuple>
struct has_type;

template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

template <typename P>
struct DirectImplications
{
    using type = std::tuple<>;
};

// To emphasize that void doesn't imply anything
template <>
struct DirectImplications<void>
{
    using type = std::tuple<>;
};

template <typename ValidatorT1, typename ValidatorT2>
struct DirectImplications<And<ValidatorT1, ValidatorT2>>
{
    using type = std::tuple<ValidatorT1, ValidatorT2>;
}; 

// https://stackoverflow.com/questions/41200299/common-types-in-two-typesets-tuples
template <typename S1, typename S2>
struct intersect
{
    template <std::size_t... Indices>
    static constexpr auto make_intersection(std::index_sequence<Indices...> ) {

        return std::tuple_cat(
            std::conditional_t<
                has_type<
                    std::tuple_element_t<Indices, S1>,
                    S2
                    >::value,
                    std::tuple<std::tuple_element_t<Indices, S1>>,
                    std::tuple<>

        >{}...);
    }
    using type = decltype(make_intersection(std::make_index_sequence<std::tuple_size<S1>::value>{}));
};

template <typename ValidatorT1, typename ValidatorT2>
struct DirectImplications<Or<ValidatorT1, ValidatorT2>>
{
    using type = std::tuple<intersect<typename DirectImplications<ValidatorT1>::type, typename DirectImplications<ValidatorT2>::type>>;
};

template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
#define IMPLIES(P, ...) \
template <> \
struct DirectImplications<argument_type<void(P)>::type> \
{ \
    using type = std::tuple<__VA_ARGS__>; \
};

template <typename P, typename Q>
constexpr bool has_edge = has_type<P, typename DirectImplications<Q>::type>::value;
template <typename P, typename Q>
constexpr bool has_bidirectional_edge = has_edge<P, Q> && has_edge<Q, P>;

IMPLIES(IsExactValue<1>, IsInClosedInterval<1, 6>)

IMPLIES(IsExactValue<2>, IsInClosedInterval<1, 6>)

IMPLIES(IsExactValue<3>, IsInClosedInterval<1, 6>)

IMPLIES(IsExactValue<4>, IsInClosedInterval<1, 6>)

IMPLIES(IsExactValue<5>, IsInClosedInterval<1, 6>)

IMPLIES(IsExactValue<6>, IsInClosedInterval<1, 6>)

IMPLIES((IsInClosedInterval<1, 6>), IsRegisterIndex)

IMPLIES(IsRegisterIndex, IsInClosedInterval<1, 6>, IsMixByte)

IMPLIES((IsInClosedInterval<0, byte_size - 1>), IsMixByte)

IMPLIES(IsMixByte, IsInClosedInterval<0, byte_size - 1>, IsMixAddress)

IMPLIES((IsInClosedInterval<0, 3999>), IsMixAddress)

IMPLIES(IsMixAddress, IsInClosedInterval<0, 3999>, IsMixPositiveWord)

IMPLIES(IsMixPositiveWord, IsMixWord, IsPositive)

IMPLIES((IsInClosedInterval<mix_int_min, mix_int_max>), IsMixWord)

IMPLIES(IsPositive, IsNonNegative)

#undef IMPLIES


template <typename P, typename Q>
struct NonTransitiveImplies
{
    static constexpr bool value = std::is_same_v<P, Q> || has_edge<P, Q>;
};

template <typename P, typename Q>
constexpr bool NonTransitiveImplies_v = NonTransitiveImplies<P, Q>::value;

template <typename ValidatorT1, typename ValidatorT2>
struct NonTransitiveImplies<And<ValidatorT1, ValidatorT2>, And<ValidatorT2, ValidatorT1>>
{
    static constexpr bool value = true;
};

template <typename ValidatorT1, typename ValidatorT2, typename ValidatorT3>
struct NonTransitiveImplies<Or<ValidatorT1, ValidatorT2>, ValidatorT3>
{
    static constexpr bool value = NonTransitiveImplies_v<ValidatorT1, ValidatorT3> && NonTransitiveImplies_v<ValidatorT2, ValidatorT3>;
};

template <typename ValidatorT1, typename ValidatorT2, typename ValidatorT3>
struct NonTransitiveImplies<ValidatorT1, And<ValidatorT2, ValidatorT3>>
{
    static constexpr bool value = NonTransitiveImplies_v<ValidatorT1, ValidatorT2> && NonTransitiveImplies_v<ValidatorT1, ValidatorT3>;
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

template <NativeInt high>
struct NonTransitiveImplies<IsInClosedInterval<0, high>, IsNonNegative>
{
    static constexpr bool value = 0 <= high;
};

template <NativeInt high>
struct NonTransitiveImplies<IsInClosedInterval<1, high>, IsPositive>
{
    static constexpr bool value = 1 <= high;
};



namespace details
{

using CommonSearchList = std::tuple<
    IsInClosedInterval<1, 6>,
    IsInClosedInterval<0, byte_size - 1>,
    IsInClosedInterval<0, 3999>
>;

template <typename to, typename from, typename LastNodeT, bool has_used_common_search_list>
consteval 
bool 
implies_impl();

template <typename to, typename from, typename LastNodeT, bool has_used_common_search_list, size_t ...Is>
consteval 
bool 
implies_direct_implication_search(std::integer_sequence<size_t, Is...> const &)
{
   return 
    (
        // If we trace the full path, the compiler will die from template recursion...
        // Thus, we only trace up to 1 node, so we can detect trivial if-and-only-if relationships.
        (
            !std::is_same_v<std::tuple_element_t<Is, typename DirectImplications<from>::type>, LastNodeT>
            and 
            implies_impl<to, std::tuple_element_t<Is, typename DirectImplications<from>::type>, from, has_used_common_search_list>()
        )

        or 
        ...
    );
}

template <typename to, typename from, typename LastNodeT, size_t ...Is>
consteval 
bool 
implies_common_search_list_search(std::integer_sequence<size_t, Is...> const &)
{
   return 
    (
        // If we trace the full path, the compiler will die from template recursion...
        // Thus, we only trace up to 1 node, so we can detect trivial if-and-only-if relationships.
        (
            NonTransitiveImplies_v<from, std::tuple_element_t<Is, CommonSearchList>>
            and
            !std::is_same_v<std::tuple_element_t<Is, CommonSearchList>, LastNodeT>
            and 
            implies_impl<to, std::tuple_element_t<Is, CommonSearchList>, from, true>()
        )
        or 
        ...
    );
}

template <typename to, typename from, typename LastNodeT, bool has_used_common_search_list>
consteval
bool
implies_impl()
{
    if constexpr (NonTransitiveImplies_v<from, to>)
        return true;

    if constexpr (!has_used_common_search_list)
    {
        if constexpr (implies_common_search_list_search<to, from, LastNodeT>(
            std::make_index_sequence<std::tuple_size_v<CommonSearchList>>()
        ))
            return true;
    }

    constexpr size_t sz = std::tuple_size_v<typename DirectImplications<from>::type>;
    if constexpr (sz == 0)
        return false;
    return
        implies_direct_implication_search<to, from, LastNodeT, has_used_common_search_list>(
            std::make_index_sequence<sz>()
        );
}

template <typename SourceT, typename FirstNodeT, typename ListT>
consteval 
bool 
implies_packed()
{
    if constexpr (ListT::size == 0) 
        return details::implies_impl<FirstNodeT, SourceT, void, false>();
    else 
    {
        using SecondNodeT = typename TypeSequenceFront<ListT>::type;
        using TailT = typename TypeSequencePopFront<ListT>::type;
        if constexpr (details::implies_impl<FirstNodeT, SourceT, void, false>() && implies_packed<FirstNodeT, SecondNodeT, TailT>())
            return true;
    
        return false;
    }
}

}

template <typename SourceT, typename FirstNodeT, typename... NodeTs>
consteval 
bool 
implies()
{
    return details::implies_packed<SourceT, FirstNodeT, TypeSequence<NodeTs...>>();
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

// This works courtesy of the `CommonSearchList`.
// Without `CommonSearchList`, this will return false.
static_assert(implies<IsExactValue<7>, IsMixPositiveWord>());

// Implies allows us to specify deduction hints directly.
// This works even without `CommonSearchList`.
static_assert(implies<
    IsExactValue<7>, 
    IsInClosedInterval<0, 3999>,
    IsMixPositiveWord
>());

static_assert(implies<IsRegisterIndex, IsInClosedInterval<0, 6>>());
static_assert(implies<IsRegisterIndex, IsInClosedInterval<0, 3099>>());

}

