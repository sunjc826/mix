#pragma once
#include <register.h>
#include <stdexcept>

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign &, Sign> Register<is_signed, size>::sign()
{
    if constexpr (is_signed)
        return arr[0].sign;
    else
        return s_plus;
}

template <bool is_signed, size_t size>
Sign Register<is_signed, size>::sign() const
{
    return REMOVE_CONST_FROM_PTR(this)->sign();
}

template <bool is_signed, size_t size>
NativeInt Register<is_signed, size>::native_sign() const
{
    return ::native_sign(sign());
}

template <bool is_signed, size_t size>
Int<is_signed, size> Register<is_signed, size>::value()
{
    return {.sp = arr};
}

template <bool is_signed, size_t size>
Int<false, Register<is_signed, size>::unsigned_size_v> Register<is_signed, size>::unsigned_value()
{
    return {.sp = std::span<Byte, unsigned_size_v>(arr.begin() + (is_signed ? 1 : 0), arr.end())};
}


template <bool is_signed, size_t size>
IntView<is_signed, size> Register<is_signed, size>::value() const
{
    return REMOVE_CONST_FROM_PTR(this)->value();
}

template <bool is_signed, size_t size>
IntView<false, Register<is_signed, size>::unsigned_size_v> Register<is_signed, size>::unsigned_value() const
{
    return REMOVE_CONST_FROM_PTR(this)->unsigned_value();
}

template <bool is_signed, size_t size>
NativeInt Register<is_signed, size>::native_value() const
{
    return value().native_value();
}

template <bool is_signed, size_t size>
NativeInt Register<is_signed, size>::native_unsigned_value() const
{
    return unsigned_value().native_value();
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::load(std::span<Byte const, size> sp)
{
    std::copy(sp.begin(), sp.end(), arr.begin());
}

template <bool is_signed, size_t size>
template <bool throw_on_overflow>
bool Register<is_signed, size>::load(NativeInt value)
{
    ByteConversionResult<size> const result = as_bytes<size>(value);
    
    if constexpr (throw_on_overflow)
    {
        if (result.overflow)
            throw std::runtime_error("overflow after conversion to bytes");
    }

    load(result.bytes);
    return result.overflow;
}

template <bool is_signed, size_t size>
template <typename EnableIfT>
EnableIfT::type Register<is_signed, size>::load(Sign sign, std::span<Byte const, size - 1> sp)
{
    arr[0].sign = sign;
    std::copy(sp.begin(), sp.end(), arr.begin() + 1);
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::store(Slice slice)
{
    FieldSpec spec = slice.spec;
    if (spec.L == 0)
    {
        spec.L = 1;
        Slice own_slice(arr, spec);
        std::copy(own_slice.sp.begin(), own_slice.sp.end(), slice.sp.begin() + 1);
        slice.sp[0] = sign();
    }
    else 
    {
        Slice own_slice(arr, spec);
        std::copy(own_slice.sp.begin(), own_slice.sp.end(), slice.sp.begin());
    }
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_left(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    size_t i;
    for (i = numerical_first_idx; i < size - shift_by; i++)
        arr[i] = arr[i + shift_by];

    for (; i < size;)
        arr[i] = 0;
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_right(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    size_t i;
    for (i = size; i --> shift_by + numerical_first_idx;)
        arr[i] = arr[i - shift_by];

    for (; i --> numerical_first_idx;)
        arr[i] = 0;
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_left_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");
    
    shift_by %= unsigned_size_v;
    std::reverse(arr.begin() + numerical_first_idx, arr.end());
    std::reverse(arr.begin(), arr.end() - shift_by);
    std::reverse(arr.end() - shift_by, arr.end());
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::shift_right_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Should be non-negative");

    shift_by %= unsigned_size_v;
    std::reverse(arr.begin() + numerical_first_idx, arr.end());
    std::reverse(arr.begin(), arr.begin() + shift_by);
    std::reverse(arr.begin() + shift_by, arr.end());
}
