#pragma once
#include <base/validated_int.decl.h>
#include <base/implies/v3.h>
#include <base/result.h>
namespace mix
{
template <typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT>
class ValidatedObject
{
    static ValidatorT validator;
protected:
    using child_type = std::conditional_t<std::is_void_v<ChildT>, ValidatedObject, ChildT>;
    using map_func_type = StorageT (*)(StorageT);
    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, type>;
    StorageT value;

    constexpr
    ValidatedObject(StorageT value) : value(value) {}

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
    template <typename OtherValidatorT, typename OtherConversionT, typename EnableIfT = std::enable_if<implies<OtherValidatorT, ValidatorT>()>>
    constexpr
    ValidatedObject(ValidatedObject<StorageT, OtherValidatorT, OtherConversionT> const &other, EnableIfT * = 0)
        : value(other.raw_unwrap())
    {}

    [[gnu::always_inline]] 
    static constexpr
    Result<child_type, void> 
    constructor(NativeInt value)
    {
        if (!validator(value)) return Result<child_type, void>::failure();
        return Result<child_type, void>::success(type(static_cast<StorageT>(value)));
    }

    [[gnu::always_inline]]
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

template <typename ValidatorT, typename ConversionT, typename ChildT = void>
class ValidatedInt : ValidatedObject<NativeInt, ValidatorT, ConversionT, ChildT>
{
public:
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

class ValidatedWord : public ValidatedInt<IsMixWord, NativeInt, ValidatedWord>
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
