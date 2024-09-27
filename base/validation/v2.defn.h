#pragma once
#include "base/types.decl.h"
#include "base/validation/validator.decl.h"
#include "base/validation/validator.impl.h"
#include <base/validation/v2.decl.h>
#include <base/implies/v3.h>
#include <base/result.h>
namespace mix
{
struct ValidatedConstructors;
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

    
    friend struct ValidatedConstructors;

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

    template <bool inplace, typename Function>
    constexpr
    ReturnIfNotInplace<inplace>
    transform_unchecked(Function fn)
    {
        if constexpr(inplace)
            value = fn(value);
        else
            return child_type(fn(value));
    }

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
    void assign(ValidatedObject<StorageT, OtherValidatorT, OtherConversionT, OtherChildT> const &other)
    {
        this->value = other.value;
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

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    increment()
    {
        return transform< [](NativeInt i) { return ++i; } >();
    }

    [[nodiscard, gnu::flatten]]
    Result<child_type, void>
    decrement()
    {
        return transform< [](NativeInt i) { return --i; } >();
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

    template <NativeInt divisor, typename T = ValidatorT>
    requires (std::is_same_v<T, IsNonNegative> && divisor > 0)
    [[gnu::always_inline]]
    child_type 
    divide() const
    {
        return this->value / divisor;
    }

    template <bool inplace, typename T = ValidatorT>
    requires (std::is_same_v<T, IsMixWord>)
    [[gnu::always_inline]]
    constexpr
    ReturnIfNotInplace<inplace>
    negate()
    {
        return this->transform_unchecked<inplace, [](NativeInt i) { return -i; } >();
    }

    
};

struct ValidatedConstructors
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
    return ValidatedObject<NativeInt, IsInClosedInterval<-1, 1>>(s);
}

template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
static
ValidatedInt<IsInClosedInterval<low + other_low, high + other_high>>
add(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedInt<IsInClosedInterval<low + other_low, high + other_high>>(lhs + rhs);
}

template <NativeInt low, NativeInt high, NativeInt other_low, NativeInt other_high>
[[nodiscard, gnu::flatten]]
static 
ValidatedInt<IsInClosedInterval<std::min({low * other_low, low * other_high, high * other_low, high * other_high}), std::max({low * other_low, low * other_high, high * other_low, high * other_high})>>
multiply(ValidatedInt<IsInClosedInterval<low, high>> lhs, ValidatedInt<IsInClosedInterval<other_low, other_high>> rhs)
{
    return ValidatedInt<IsInClosedInterval<std::min({low * other_low, low * other_high, high * other_low, high * other_high}), std::max({low * other_low, low * other_high, high * other_low, high * other_high})>>(lhs * rhs);
}

};

template <NativeInt literal>
ValidatedInt<IsInClosedInterval<literal, literal>>
to_interval(ValidatedInt<IsExactValue<literal>> i)
{
    return ValidatedInt<IsInClosedInterval<literal, literal>>(i);
}

inline
ValidatedInt<IsInClosedInterval<0, byte_size - 1>>
to_interval(ValidatedInt<IsMixByte> i)
{
    return ValidatedInt<IsInClosedInterval<0, byte_size - 1>>(i);
}


}
