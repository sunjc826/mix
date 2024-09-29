#pragma once
#include "base/types.decl.h"
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
    return 1 <= i && i <= 6;
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
struct IsExactValue : public IntValidatorToFunctor<is_exact_value<literal>> {};

using IsPositive = IntValidatorToFunctor<is_positive>;

using IsNegative = IntValidatorToFunctor<is_negative>;

using IsNonNegative = IntValidatorToFunctor<is_nonnegative>;

using IsNonPositive = IntValidatorToFunctor<is_nonpositive>;

template <NativeInt low, NativeInt high>
struct IsInClosedInterval : public IntValidatorToFunctor<is_in_closed_interval<low, high>> {};

using IsMixByte = IntValidatorToFunctor<is_mix_byte>;

using IsRegisterIndex = IntValidatorToFunctor<is_register_index>;

using IsMixAddress = IntValidatorToFunctor<is_mix_address>;

using IsMixWord = IntValidatorToFunctor<is_mix_word>;

using IsMixPositiveWord = IntValidatorToFunctor<is_mix_positive_word>;

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
struct IsNonEmpty : public IntValidatorToFunctor<is_nonempty<ContainerT>> {};

template <typename ContainerT, typename ValidatorT>
requires has_size_method<ContainerT>
[[gnu::always_inline]]
inline constexpr
bool
custom_size_predicate(PassByValueOrRef<ContainerT> container)
{
    return ValidatorT()(container.size());
}

template <typename ContainerT, typename ValidatorT>
requires has_size_method<ContainerT>
struct CustomSizePredicate : public ValidatorToFunctor<ContainerT, custom_size_predicate<ContainerT, ValidatorT>> {};


}
