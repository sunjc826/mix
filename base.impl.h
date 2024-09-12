#pragma once
#include <base.h>
#include <stdexcept>

template <NativeInt value>
constexpr
bool
is_exact_value(NativeInt i)
{
    return value == i;
}

template <NativeInt lower_bound, NativeInt upper_bound>
constexpr
bool
is_in_closed_interval(NativeInt i)
{
    static_assert(lower_bound <= upper_bound);
    return lower_bound <= i && i < upper_bound;
}

constexpr
bool 
is_mix_byte(NativeInt i)
{
    return 0 <= i && i < byte_size;
}

constexpr
bool 
is_register_index(NativeInt i)
{
    return 0 <= i && i <= 6;
}

constexpr
bool 
is_mix_address(NativeInt i)
{
    return 0 <= i && i < main_memory_size;
}

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
constexpr 
std::array<NativeInt, numerical_bytes_in_extended_word + 1> 
pow_lookup_table(NativeByte base)
{
    std::array<NativeInt, numerical_bytes_in_extended_word + 1> lut;
    lut[0] = 1;
    for (size_t i = 1; i < lut.size(); i++)
        lut[i] = lut[i - 1] * base;
    return lut;
}

constexpr auto lut = pow_lookup_table(byte_size);

static __attribute__((always_inline))
constexpr 
NativeInt
pow(NativeByte base, size_t exponent)
{
    return pow_lookup_table(base)[exponent];
}

template <size_t exponent>
static __attribute__((always_inline))
constexpr 
NativeInt
pow(NativeByte base)
{
    static_assert(0 <= exponent && exponent <= 2 * numerical_bytes_in_word);
    return pow(base, exponent);
}

// TODO: change to use Result datatype instead of throwing exceptions
void 
check_address_bounds(NativeInt value)
{
    if (value < 0)
        throw std::runtime_error("Negative address");
    else if (value >= main_memory_size)
        throw std::runtime_error("Overflowing address");
}

// Every negative MIX integral value must be representable by NativeInt
static_assert(-(lut.back() - 1) >= std::numeric_limits<NativeInt>::min());
// Every positive MIX integral value must be representable by NativeInt
static_assert(lut.back() - 1 <= std::numeric_limits<NativeInt>::max());

constexpr
bool 
is_mix_word(NativeInt i)
{
    return -lut[numerical_bytes_in_word] < i && i < lut[numerical_bytes_in_word];
}

constexpr
bool
is_mix_positive_word(NativeInt i)
{
    return 0 < i && i < lut[numerical_bytes_in_word];
}

constexpr 
NativeInt 
native_sign(Sign sign)
{
    return sign == s_plus ? 1 : -1; 
}

constexpr 
Sign
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

std::array<Byte, bytes_in_word> as_bytes(ValidatedWord word)
{
    NativeInt value = word;
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

    std::array<Byte, bytes_in_word> result;
    for (size_t s = result.size(); s --> 1;)
    {
        result[s].byte = value % byte_size;
        value /= byte_size;
    }

    result[0].sign = sign;

    return result;
}
