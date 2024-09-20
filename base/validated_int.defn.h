#pragma once
#include <base/validated_int.decl.h>
#include <base/implies/v2.h>

template <typename StorageT, bool (*validator)(NativeInt), typename ChildT>
class ValidatedInt
{
    using type = std::conditional_t<std::is_void_v<ChildT>, ValidatedInt, ChildT>;
protected:
    NativeInt value;
    constexpr
    ValidatedInt(StorageT value) : value(value) {}
public:
    template <typename OtherStorageT, bool (*other_validator)(NativeInt), typename EnableIfT = std::enable_if<implies<other_validator, validator>()>>
    constexpr
    ValidatedInt(ValidatedInt<OtherStorageT, other_validator> other, EnableIfT * = 0)
        : value(other.raw_native_int())
    {}

    static __attribute__((always_inline))
    constexpr
    Result<type, void> 
    constructor(NativeInt value)
    {
        if (!validator(value)) return Result<type, void>::failure();
        return Result<type, void>::success(type(static_cast<StorageT>(value)));
    }

    __attribute__((always_inline))
    NativeInt raw_native_int() const
    {
        return value;
    }

    __attribute__((always_inline))
    constexpr
    StorageT unwrap() const
    {
        return value;    
    }

    __attribute__((always_inline))
    constexpr
    operator StorageT() const
    {
        return value;
    }
};

class ValidatedWord : public ValidatedInt<NativeInt, is_mix_word, ValidatedWord>
{
public:
    __attribute__((always_inline))
    void negate()
    {
        value = -value;
    }
};
