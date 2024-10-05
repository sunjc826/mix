#pragma once
#include "base/validation/validator.impl.h"
#include "config.impl.h"
#include <vm/register.defn.h>

#include <stdexcept>
namespace mix
{

template <size_t size>
IntMutable<false, size> RegisterWithoutSign<size>::unsigned_value() 
{
    return IntMutable<false, size>(std::span<Byte, size>(reg.begin(), reg.end()));
}

template <size_t size>
IntView<false, size> RegisterWithoutSign<size>::unsigned_value() const
{
    return REMOVE_CONST_FROM_PTR(this)->unsigned_value();
}

template <size_t size>
ValidatedInt<IsInClosedInterval<0, lut[size] - 1>> RegisterWithoutSign<size>::native_unsigned_value() const
{
    return unsigned_value().native_value();
}

template <size_t size>
void RegisterWithoutSign<size>::store(Sign sign, SliceMutable slice) const
{
}

template <bool is_signed, size_t size>
template <typename>
requires (is_signed)
RegisterWithoutSign<size - 1> Register<is_signed, size>::unsigned_register() const
{
    return RegisterWithoutSign<size - 1>(std::span<Byte, size - 1>(reg.begin() + 1, reg.end()));
}

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign &, Sign> Register<is_signed, size>::sign()
{
    if constexpr (is_signed)
        return reg[0].sign;
    else
        return s_plus;
}

template <bool is_signed, size_t size>
Sign Register<is_signed, size>::sign() const
{
    return REMOVE_CONST_FROM_PTR(this)->sign();
}

template <bool is_signed, size_t size>
ValidatedInt<IsInClosedInterval<-1, 1>> Register<is_signed, size>::native_sign() const
{
    return ValidatedUtils::from_sign(sign());
}

template <bool is_signed, size_t size>
IntMutable<is_signed, size>
Register<is_signed, size>::value()
{
    return IntMutable<is_signed, size>(reg);
}

template <bool is_signed, size_t size>
IntMutable<false, Register<is_signed, size>::unsigned_size_v>
Register<is_signed, size>::unsigned_value()
{
    return IntMutable<false, Register<is_signed, size>::unsigned_size_v>(std::span<Byte, unsigned_size_v>(reg.begin() + (is_signed ? 1 : 0), reg.end()));
}

template <bool is_signed, size_t size>
IntView<is_signed, size>
Register<is_signed, size>::value() const
{
    return REMOVE_CONST_FROM_PTR(this)->value();
}

template <bool is_signed, size_t size>
IntView<false, Register<is_signed, size>::unsigned_size_v>
Register<is_signed, size>::unsigned_value() const
{
    return REMOVE_CONST_FROM_PTR(this)->unsigned_value();
}

template <bool is_signed, size_t size>
ValidatedInt<IsInClosedInterval<-(lut[Register<is_signed, size>::unsigned_size_v] - 1), lut[Register<is_signed, size>::unsigned_size_v] - 1>> 
Register<is_signed, size>::native_value() const
{
    return value().native_value();
}

template <bool is_signed, size_t size>
ValidatedInt<IsInClosedInterval<0, lut[Register<is_signed, size>::unsigned_size_v] - 1>> 
Register<is_signed, size>::native_unsigned_value() const
{
    return unsigned_value().native_value();
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::load(std::span<Byte const, size> sp)
{
    std::copy(sp.begin(), sp.end(), reg.begin());
}

template <bool is_signed, size_t size>
template <bool throw_on_overflow>
std::conditional_t<throw_on_overflow, void, bool> Register<is_signed, size>::load(NativeInt value)
{
    ByteConversionResult<size> const result = as_bytes<size>(value);
    
    if constexpr (throw_on_overflow)
    {
        if (result.overflow)
            throw std::runtime_error("overflow after conversion to bytes");
    }

    load(result.bytes);
    if constexpr (!throw_on_overflow)
        return result.overflow;
}

template <bool is_signed, size_t size>
template <typename>
requires (is_signed)
void Register<is_signed, size>::load(Sign sign, std::span<Byte const, size - 1> sp)
{
    reg[0].sign = sign;
    std::copy(sp.begin(), sp.end(), reg.begin() + 1);
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::store(SliceMutable slice) const
{
    if (slice.is_signed())
    {
        slice.sp[0].sign = reg.sign();
        std::copy(reg.end() - (slice.length() - 1), reg.end(), slice.sp.begin() + 1);
    }
    else
    {
        std::copy(reg.end() - slice.length(), reg.end(), slice.sp.begin());
    }
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_left(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    size_t i;
    for (i = numerical_first_idx; i < size - shift_by; i++)
        reg[i] = reg[i + shift_by];

    for (; i < size;)
        reg[i] = zero_byte;
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_right(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    size_t i;
    for (i = size; i --> shift_by + numerical_first_idx;)
        reg[i] = reg[i - shift_by];

    for (; i --> numerical_first_idx;)
        reg[i] = zero_byte;
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_left_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");
    
    shift_by %= unsigned_size_v;
    std::reverse(reg.begin() + numerical_first_idx, reg.end());
    std::reverse(reg.begin(), reg.end() - shift_by);
    std::reverse(reg.end() - shift_by, reg.end());
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_right_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    shift_by %= unsigned_size_v;
    std::reverse(reg.begin() + numerical_first_idx, reg.end());
    std::reverse(reg.begin(), reg.begin() + shift_by);
    std::reverse(reg.begin() + shift_by, reg.end());
}

}
