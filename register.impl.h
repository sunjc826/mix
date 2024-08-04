#pragma once
#include <register.h>
#include <machine.h>
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
void Register<is_signed, size>::store(std::span<Byte const, size> sp)
{
    std::copy(sp.begin(), sp.end(), arr.begin());
}

template <bool is_signed, size_t size>
template <OverflowPolicy overflow_policy>
void Register<is_signed, size>::store(NativeInt value)
{
    ByteConversionResult<size> const bytes = as_bytes<size>(value);
    if constexpr (overflow_policy == OverflowPolicy::set_overflow_bit)
        m.overflow = true;
    else if constexpr (overflow_policy == OverflowPolicy::do_nothing)
        ;
    else if constexpr (overflow_policy == OverflowPolicy::throw_exception)
        throw std::runtime_error("overflow after conversion to bytes");
    else
        throw std::runtime_error("case unaccounted for");
    store(bytes);
}

template <bool is_signed, size_t size>
template <typename EnableIfT>
EnableIfT::type Register<is_signed, size>::store(Sign sign, std::span<Byte const, size - 1> sp)
{
    arr[0].sign = sign;
    std::copy(sp.begin(), sp.end(), arr.begin() + 1);
}
