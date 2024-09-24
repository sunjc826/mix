#pragma once
#include <base/validation/v1.decl.h>
#include <base/implies/v2.h>
#include <base/result.h>
namespace mix
{

template <bool (*validator)(NativeInt), typename ConversionT, typename ChildT>
class ValidatedInt
{
protected:
    using child_type = std::conditional_t<std::is_void_v<ChildT>, ValidatedInt, ChildT>;
    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, child_type>;
    NativeInt value;

    constexpr
    ValidatedInt(NativeInt value) : value(value) {}

    template <bool inplace, NativeInt (*fn)(NativeInt)>
    constexpr
    ReturnIfNotInplace<inplace>
    transform_unchecked()
    {
        if constexpr(inplace)
            value = fn(value);
        else
            return ValidatedInt(fn(value));
    }

    template <bool inplace, typename Function>
    constexpr
    ReturnIfNotInplace<inplace>
    transform_unchecked(Function fn)
    {
        if constexpr(inplace)
            value = fn(value);
        else
            return ValidatedInt(fn(value));
    }

public:
    template <bool (*other_validator)(NativeInt), typename OtherConversionT, typename EnableIfT = std::enable_if<implies<other_validator, validator>()>>
    constexpr
    ValidatedInt(ValidatedInt<other_validator, OtherConversionT> other, EnableIfT * = 0)
        : value(other.raw_native_int())
    {}

    [[gnu::always_inline]] 
    static constexpr
    Result<child_type, void> 
    constructor(NativeInt value)
    {
        if (!validator(value)) return Result<child_type, void>::failure();
        return Result<child_type, void>::success(child_type(value));
    }

    [[gnu::always_inline]]
    NativeInt raw_native_int() const
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

    template <NativeInt (*fn)(NativeInt)>
    [[nodiscard]]
    constexpr
    Result<child_type, void> 
    transform()
    {
        return ValidatedInt::constructor(fn(value));
    }

    template <typename Function>
    [[nodiscard]]
    constexpr
    Result<child_type, void> 
    transform(Function fn)
    {
        return ValidatedInt::constructor(fn(value));
    }

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
        return transform< [](NativeInt i) { return --i; }>();
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
};

class ValidatedWord : public ValidatedInt<is_mix_word, NativeInt, ValidatedWord>
{
public:
    [[gnu::always_inline]]
    constexpr
    ValidatedWord(ValidatedInt const &obj)
        : ValidatedInt(obj)
    {}

    template <bool inplace>
    [[gnu::always_inline]]
    constexpr
    ReturnIfNotInplace<inplace> 
    negate()
    {
        return transform_unchecked<inplace, [](NativeInt i) { return -i; } >();
    }
};

}
