#pragma once
#include <base/validation/v1.decl.h>
#include <base/implies/v2.h>
#include <base/result.h>
namespace mix
{

template <typename StorageT, bool (*validator)(NativeInt), typename ChildT>
class ValidatedInt
{
protected:
    using type = std::conditional_t<std::is_void_v<ChildT>, ValidatedInt, ChildT>;
    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, type>;
    NativeInt value;

    constexpr
    ValidatedInt(StorageT value) : value(value) {}

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
    template <typename OtherStorageT, bool (*other_validator)(NativeInt), typename EnableIfT = std::enable_if<implies<other_validator, validator>()>>
    constexpr
    ValidatedInt(ValidatedInt<OtherStorageT, other_validator> other, EnableIfT * = 0)
        : value(other.raw_native_int())
    {}

    [[gnu::always_inline]] static 
    constexpr
    Result<type, void> 
    constructor(NativeInt value)
    {
        if (!validator(value)) return Result<type, void>::failure();
        return Result<type, void>::success(type(static_cast<StorageT>(value)));
    }

    [[gnu::always_inline]]
    NativeInt raw_native_int() const
    {
        return value;
    }

    [[gnu::always_inline]]
    constexpr
    StorageT unwrap() const
    {
        return value;    
    }

    [[gnu::always_inline]]
    constexpr
    operator StorageT() const
    {
        return value;
    }

    template <NativeInt (*fn)(NativeInt)>
    [[nodiscard]]
    constexpr
    Result<type, void> 
    transform()
    {
        return ValidatedInt::constructor(fn(value));
    }

    template <typename Function>
    [[nodiscard]]
    constexpr
    Result<type, void> 
    transform(Function fn)
    {
        return ValidatedInt::constructor(fn(value));
    }

    [[nodiscard, gnu::flatten]]
    Result<type, void>
    increment()
    {
        return transform< [](NativeInt i) { return ++i; } >();
    }

    [[nodiscard, gnu::flatten]]
    Result<type, void>
    decrement()
    {
        return transform< [](NativeInt i) { return --i; }>();
    }

    [[nodiscard, gnu::flatten]]
    Result<type, void>
    add(NativeInt other)
    {
        return transform([other](NativeInt i) { return i + other; });
    }

    [[nodiscard, gnu::flatten]]
    Result<type, void>
    subtract(NativeInt other)
    {
        return transform([other](NativeInt i) { return i - other; });
    }

    [[nodiscard, gnu::flatten]]
    Result<type, void>
    multiply(NativeInt other)
    {
        return transform([other](NativeInt i) { return i + other; });
    }
};

class ValidatedWord : public ValidatedInt<NativeInt, is_mix_word, ValidatedWord>
{
public:
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
