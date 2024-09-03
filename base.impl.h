#pragma once
#include <base.h>
#include <stdexcept>
// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
static __attribute__((always_inline))
constexpr std::array<NativeInt, bytes_in_extended_word> 
pow_lookup_table(NativeByte base)
{
    std::array<NativeInt, bytes_in_extended_word> lut;
    lut[0] = 1;
    for (size_t i = 1; i < lut.size(); i++)
        lut[i] = lut[i - 1] * base;
    return lut;
}

constexpr auto lut = pow_lookup_table(byte_size);

static __attribute__((always_inline))
constexpr NativeInt
pow(NativeByte base, size_t exponent)
{
    return pow_lookup_table(base)[exponent];
}

template <size_t exponent>
static __attribute__((always_inline))
constexpr NativeInt
pow(NativeByte base)
{
    static_assert(0 <= exponent && exponent <= 2 * numerical_bytes_in_word);
    return pow(base, exponent);
}

void check_address_bounds(NativeInt value)
{
    
    // if (value < 0)
        // throw std::runtime_error("Negative address");
    // else if (value >= main_memory_size)
    //     throw std::runtime_error("Overflowing address");
}

// Every negative MIX integral value must be representable by NativeInt
static_assert(-(lut.back() - 1) >= std::numeric_limits<NativeInt>::min());
// Every positive MIX integral value must be representable by NativeInt
static_assert(lut.back() - 1 <= std::numeric_limits<NativeInt>::max());

static __attribute__((always_inline))
constexpr NativeInt 
native_sign(Sign sign)
{
    return sign == s_plus ? 1 : -1; 
}

static __attribute__((always_inline))
constexpr Sign
sign(NativeInt value)
{
    if (value == 0)
        throw std::runtime_error("Unknown sign");
    return value > 0 ? s_plus : s_minus;
}

template <OwnershipKind kind, bool is_signed, size_t size>
NativeInt IntegralContainer<kind, is_signed, size>::native_sign() const
{
    return ::native_sign(sign());
}

template <OwnershipKind kind, bool is_signed, size_t size>
template <typename EnableIfT>
EnableIfT::type IntegralContainer<kind, is_signed, size>::sign()
{
    if constexpr(is_signed)
        return container[0].sign;
    else
        return s_plus;
}

template <OwnershipKind kind, bool is_signed, size_t size>
std::conditional_t<is_signed, Sign const &, Sign> IntegralContainer<kind, is_signed, size>::sign() const
{
    if constexpr(is_signed)
        return container[0].sign;
    else
        return s_plus;
}

template <OwnershipKind kind, bool is_signed, size_t size>
NativeInt IntegralContainer<kind, is_signed, size>::native_value() const
{
    NativeInt accum = 0;
    for (size_t i = is_signed; i < container.size(); i++)
        accum += lut[i] * container[i].byte;
    return native_sign() * accum;
}

template <size_t size>
ByteConversionResult<size> as_bytes(NativeInt value)
{
    Sign sign;
    if (value < 0)
    {
        sign = s_minus;
        value = -value;
    }
    else
    {
        sign = s_plus;
    }

    ByteConversionResult<size> result;
    for (size_t s = size; s --> 1;)
    {
        result.bytes[s].byte = value % byte_size;
        value /= byte_size;
    }

    result.bytes[0].sign = sign;
    
    result.overflow = value > 0;

    return result;
}
