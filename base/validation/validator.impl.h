#pragma once
#include <base/function.h>
#include <base/validation/validator.decl.h>
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

constexpr
bool
is_positive(NativeInt i)
{
    return i > 0;
}

constexpr
bool
is_negative(NativeInt i)
{
    return i < 0;
}

constexpr
bool
is_nonnegative(NativeInt i)
{
    return i >= 0;
}

constexpr
bool
is_nonpositive(NativeInt i)
{
    return i <= 0;
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

template <NativeInt literal>
struct IsExactValue : public ValidatorToFunctor<is_exact_value<literal>> {};

using IsPositive = ValidatorToFunctor<is_positive>;

using IsNegative = ValidatorToFunctor<is_negative>;

using IsNonNegative = ValidatorToFunctor<is_nonnegative>;

using IsNonPositive = ValidatorToFunctor<is_nonpositive>;

template <NativeInt low, NativeInt high>
struct IsInClosedInterval : public ValidatorToFunctor<is_in_closed_interval<low, high>> {};

using IsMixByte = ValidatorToFunctor<is_mix_byte>;

using IsRegisterIndex = ValidatorToFunctor<is_register_index>;

using IsMixAddress = ValidatorToFunctor<is_mix_address>;

using IsMixWord = ValidatorToFunctor<is_mix_word>;

using IsMixPositiveWord = ValidatorToFunctor<is_mix_positive_word>;

template <typename ContainerT>
requires has_empty_method<ContainerT>
constexpr
bool
is_nonempty(ContainerT container)
{
    return !container.empty();
}

template <typename ContainerT>
requires has_empty_method<ContainerT>
struct IsNonEmpty : public ValidatorToFunctor<is_nonempty<ContainerT>> {};

template <typename ContainerT, typename ValidatorT>
requires has_size_method<ContainerT>
[[gnu::always_inline]]
static constexpr
bool
custom_size_predicate(ContainerT const &container)
{
    static ValidatorT const validator;
    return validator(container.size());
}

template <typename ContainerT, typename ValidatorT>
requires has_size_method<ContainerT>
struct CustomSizePredicate : public ValidatorToFunctor<custom_size_predicate<ContainerT, ValidatorT>> {};


}
