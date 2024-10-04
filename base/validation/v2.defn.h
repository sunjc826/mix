#pragma once
#include "base/type_sequence.h"
#include "base/types.decl.h"
#include "base/validation/validator.decl.h"
#include "base/validation/validator.impl.h"
#include <base/validation/v2.decl.h>
#include <base/implies/v3.h>
#include <base/result.h>
#include <string_view>
#include <type_traits>
namespace mix
{
struct ValidatedUtils;
template <typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT>
class ValidatedObject
{
protected:
    static inline ValidatorT const validator;
    using child_type = std::conditional_t<std::is_void_v<ChildT>, ValidatedObject, ChildT>;
    using map_func_type = StorageT (*)(StorageT);
    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, child_type>;
    StorageT value;
    
    constexpr
    explicit ValidatedObject(StorageT value) : value(value) {}

    
    friend struct ValidatedUtils;

    template <bool inplace, map_func_type fn>
    constexpr
    ReturnIfNotInplace<inplace>
    transform_unchecked()
    {
        if constexpr(inplace)
            value = fn(value);
        else
            return child_type(fn(value));
    }

    // template <bool inplace, typename Function>
    // constexpr
    // ReturnIfNotInplace<inplace>
    // transform_unchecked(Function fn)
    // {
    //     if constexpr(inplace)
    //         value = fn(value);
    //     else
    //         return child_type(fn(value));
    // }

public:
    template <typename OtherValidatorT, typename OtherConversionT, typename OtherChildT>
    requires (implies<OtherValidatorT, ValidatorT>())
    [[gnu::always_inline]]
    constexpr
    ValidatedObject(ValidatedObject<StorageT, OtherValidatorT, OtherConversionT, OtherChildT> const &other)
        : value(other.raw_unwrap())
    {}

    [[gnu::always_inline]] 
    static constexpr
    Result<child_type, void> 
    constructor(StorageT value)
    {
        if (!validator(value)) return Result<child_type, void>::failure();
        return Result<child_type, void>::success(ValidatedObject(value));
    }

    template <typename ...HintsT, typename OtherValidatorT, typename OtherConversionT, typename OtherChildT>
    requires (implies<OtherValidatorT, HintsT..., ValidatorT>())
    ValidatedObject & 
    operator=(ValidatedObject<StorageT, OtherValidatorT, OtherConversionT, OtherChildT> const &other)
    {
        this->value = other.value;
        return *this;
    }

    [[gnu::always_inline]]
    constexpr
    StorageT raw_unwrap() const
    {
        return value;
    }

    [[gnu::always_inline]]
    constexpr
    ConversionT unwrap() const
    {
        return value;    
    }

    [[gnu::always_inline]]
    constexpr
    operator ConversionT() const
    {
        return value;
    }

    // operator ValidatedObject<StorageT, ValidatorT>() const
    // {
    //     return this->raw_unwrap();
    // }

    template <map_func_type fn>
    [[nodiscard]]
    constexpr
    Result<child_type, void> 
    transform()
    {
        return child_type::constructor(fn(value));
    }

    template <typename Function>
    [[nodiscard]]
    constexpr
    Result<child_type, void> 
    transform(Function fn)
    {
        return child_type::constructor(fn(value));
    }
};

template <typename ValidatorT, typename ConversionT, typename ChildT>
class ValidatedInt : public ValidatedObject<NativeInt, ValidatorT, ConversionT, std::conditional_t<std::is_void_v<ChildT>, ValidatedInt<ValidatorT, ConversionT, void>, ChildT>>
{
    using parent_type = ValidatedObject<NativeInt, ValidatorT, ConversionT, std::conditional_t<std::is_void_v<ChildT>, ValidatedInt<ValidatorT, ConversionT, void>, ChildT>>;
    using child_type = parent_type::child_type;

    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, child_type>;
public:
    template <typename OtherValidatorT, typename OtherConversionT, typename OtherChildT>
    // We need the requires here so that the compiler error when implies fails appears at construction
    requires (implies<OtherValidatorT, ValidatorT>())
    [[gnu::always_inline, gnu::flatten]]
    constexpr
    ValidatedInt(ValidatedObject<NativeInt, OtherValidatorT, OtherConversionT, OtherChildT> const &obj)
        : /* otherwise the compiler error would appear here ---> */ parent_type(obj)
    {}

    template <typename OtherValidatorT, typename OtherConversionT, typename OtherChildT>
    // We need the requires here so that the compiler error when implies fails appears at construction
    requires (implies<OtherValidatorT, ValidatorT>())
    [[gnu::always_inline, gnu::flatten]]
    constexpr
    ValidatedInt &
    operator=(ValidatedObject<NativeInt, OtherValidatorT, OtherConversionT, OtherChildT> const &obj)
    {
        this->parent_type::operator=(obj);
        return *this;
    }
    
    // using ValidatedObject<NativeInt, ValidatorT, ConversionT, std::conditional_t<std::is_void_v<ChildT>, ValidatedInt<ValidatorT, ConversionT, void>, ChildT>>::ValidatedObject;

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    increment()
    {
        return this->template transform<[](NativeInt i) { return ++i; }>();
    }

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    decrement()
    {
        return this->template transform< [](NativeInt i) { return --i; } >();
    }

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    add(NativeInt other)
    {
        return transform([other](NativeInt i) { return i + other; });
    }

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    subtract(NativeInt other)
    {
        return transform([other](NativeInt i) { return i - other; });
    }

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    multiply(NativeInt other)
    {
        return transform([other](NativeInt i) { return i + other; });
    }

    template <bool inplace, typename T = ValidatorT>
    requires (std::is_same_v<T, IsMixWord>)
    [[gnu::always_inline]]
    constexpr
    ReturnIfNotInplace<inplace>
    negate()
    {
        return this->template transform_unchecked<inplace, [](NativeInt i) { return -i; } >();
    }
};

struct ValidatedUtils
{

template <NativeInt mod_class>
requires (is_positive(mod_class))
[[gnu::always_inline, gnu::flatten]]
static
ValidatedInt<IsInClosedInterval<0, mod_class - 1>>
from_mod(ValidatedNonNegative i)
{
    return ValidatedObject<NativeInt, IsInClosedInterval<0, mod_class - 1>>(i.raw_unwrap() % mod_class);
}

static
std::tuple<Sign, ValidatedInt<IsNonNegative>>
from_abs(NativeInt i)
{
    if (i < 0)
        return {s_minus, ValidatedObject<NativeInt, IsNonNegative>(-i)};
    else
        return {s_plus, ValidatedObject<NativeInt, IsNonNegative>(i)};
}

static
ValidatedInt<IsInClosedInterval<-1, 1>>
from_sign(Sign s)
{
    return ValidatedObject<NativeInt, IsInClosedInterval<-1, 1>>(s == s_plus ? 1 : -1);
}

template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
static
ValidatedInt<IsInClosedInterval<low + other_low, high + other_high>>
add(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedObject<NativeInt, IsInClosedInterval<low + other_low, high + other_high>>(lhs + rhs);
}

template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
static 
ValidatedInt<IsInClosedInterval<std::min({low * other_low, low * other_high, high * other_low, high * other_high}), std::max({low * other_low, low * other_high, high * other_low, high * other_high})>>
multiply(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedObject<NativeInt, IsInClosedInterval<std::min({low * other_low, low * other_high, high * other_low, high * other_high}), std::max({low * other_low, low * other_high, high * other_low, high * other_high})>>(lhs.raw_unwrap() * rhs.raw_unwrap());
}

static
ValidatedNonNegative
add(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedObject<NativeInt, IsNonNegative>(lhs + rhs);
}

static
ValidatedNonNegative
multiply(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedObject<NativeInt, IsNonNegative>(lhs * rhs);
}

static
ValidatedNonNegative
divide(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedObject<NativeInt, IsNonNegative>(lhs / rhs);
}

static
ValidatedNonNegative
modulo(ValidatedNonNegative lhs, ValidatedPositive rhs)
{
    return ValidatedObject<NativeInt, IsNonNegative>(lhs % rhs);
}

template <typename Func1, typename Func2, typename StorageT, typename ValidatorT1, typename ValidatorT2, typename ConversionT, typename ChildT>
static
std::common_type_t<
    decltype(std::declval<Func1>()(std::declval<ValidatedObject<StorageT, ValidatorT1>>())),
    decltype(std::declval<Func2>()(std::declval<ValidatedObject<StorageT, ValidatorT2>>()))
>
visit(
    ValidatedObject<StorageT, Or<ValidatorT1, ValidatorT2>, ConversionT, ChildT> obj,
    Func1 func1,
    Func2 func2
)
{
    if (ValidatorT1()(obj))
        return func1(ValidatedObject<StorageT, ValidatorT1>(obj));
    else
        return func2(ValidatedObject<StorageT, ValidatorT2>(obj));
}

};


template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
ValidatedInt<IsInClosedInterval<low + other_low, high + other_high>>
operator+(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedUtils::add(lhs, rhs);
}

template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
decltype(auto)
operator*(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedUtils::multiply(lhs, rhs);
}

inline
decltype(auto)
operator+(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedUtils::add(lhs, rhs);
}

inline
decltype(auto)
operator*(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedUtils::multiply(lhs, rhs);
}

inline
decltype(auto)
operator%(ValidatedNonNegative lhs, ValidatedPositive rhs)
{
    return ValidatedUtils::modulo(lhs, rhs);
}

inline
decltype(auto)
operator/(ValidatedNonNegative lhs, ValidatedNonNegative rhs)
{
    return ValidatedUtils::divide(lhs, rhs);
}

template <NativeInt literal>
constexpr
ValidatedInt<IsInClosedInterval<literal, literal>>
to_interval(ValidatedInt<IsExactValue<literal>> i)
{
    return ValidatedInt<IsInClosedInterval<literal, literal>>(i);
}

inline
constexpr
ValidatedInt<IsInClosedInterval<0, byte_size - 1>>
to_interval(ValidatedInt<IsMixByte> i)
{
    return ValidatedInt<IsInClosedInterval<0, byte_size - 1>>(i);
}

template <NativeInt literal>
constexpr
ValidatedLiteral<literal>
from_literal()
{
    return ValidatedLiteral<literal>::constructor(literal).value();
}

template <typename DestinationT, typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT>
requires (implies<ValidatorT, DestinationT>())
constexpr
ValidatedObject<StorageT, DestinationT>
deduce(ValidatedObject<StorageT, ValidatorT, ConversionT, ChildT> i)
{
    return ValidatedObject<StorageT, DestinationT>(i);
}

namespace details
{
    template <typename ListT, typename StorageT, typename ValidatorT>
    requires (ListT::size > 0)
    constexpr
    ValidatedObject<StorageT, typename TypeSequenceBack<ListT>::type>
    deduce_sequence(ValidatedObject<StorageT, ValidatorT> i)
    {
        if constexpr (ListT::size == 1) 
            return deduce<typename TypeSequenceFront<ListT>::type, StorageT, ValidatorT>(i);
        else
            return deduce_sequence<typename TypeSequencePopFront<ListT>::type, StorageT, ValidatorT>(ValidatedObject<StorageT, typename TypeSequenceFront<ListT>::type>(i));
    }

}

template <typename ListT, typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT>
requires (ListT::size > 0)
constexpr
decltype(auto)
deduce_sequence(ValidatedObject<StorageT, ValidatorT, ConversionT, ChildT> i)
{
    return details::deduce_sequence<ListT, StorageT, ValidatorT>(ValidatedObject<StorageT, ValidatorT>(i));
}

template <typename T>
inline constexpr bool is_trivial_for_purposes_of_calls = 
    std::is_trivially_copy_constructible_v<T> &&
    std::is_trivially_move_constructible_v<T> &&
    std::is_trivially_destructible_v<T>;

static_assert(is_trivial_for_purposes_of_calls<ValidatedObject<std::string_view, CustomSizePredicate<std::string_view, IsExactValue<5>>>>);
static_assert(is_trivial_for_purposes_of_calls<ValidatedWord>);
}
