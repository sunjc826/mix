#pragma once
#include <base/validation/v2.decl.h>
#include <base/implies/v3.h>
#include <base/result.h>
namespace mix
{
template <typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT>
class ValidatedObject
{
protected:
    static ValidatorT validator;
    using child_type = std::conditional_t<std::is_void_v<ChildT>, ValidatedObject, ChildT>;
    using map_func_type = StorageT (*)(StorageT);
    template <bool inplace>
    using ReturnIfNotInplace = std::conditional_t<inplace, void, child_type>;
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
    template <typename OtherValidatorT, typename OtherConversionT>
    requires (implies<OtherValidatorT, ValidatorT>())
    [[gnu::always_inline]]
    constexpr
    ValidatedObject(ValidatedObject<StorageT, OtherValidatorT, OtherConversionT> const &other)
        : value(other.raw_unwrap())
    {}

    [[gnu::always_inline]] 
    static constexpr
    Result<child_type, void> 
    constructor(StorageT value)
    {
        if (!validator(value)) return Result<child_type, void>::failure();
        return Result<child_type, void>::success(child_type(value));
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
public:
    [[gnu::always_inline]]
    constexpr
    ValidatedInt(parent_type const &obj)
        : parent_type(obj)
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
};

class ValidatedWord : public ValidatedInt<IsMixWord, NativeInt, ValidatedWord>
{
    using parent_type = ValidatedInt<IsMixWord, NativeInt, ValidatedWord>;
public:
    [[gnu::always_inline]]
    constexpr
    ValidatedWord(ValidatedObject const &obj)
        : parent_type(obj)
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
