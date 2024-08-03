#include <algorithm>
#include <cmath>
#include <config.h>
#include <functional>
#include <optional>
#include <stdexcept>
#include <utilities.h>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <span>
#include <type_traits>
#include <tuple>

#include <machine.decl.h>

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
static __attribute__((always_inline))
constexpr std::array<NativeInt, 2 * numerical_bytes_in_word + 1> 
pow_lookup_table(NativeByte base)
{
    std::array<NativeInt, 2 * numerical_bytes_in_word + 1> lut;
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

static __attribute__((always_inline))
void check_address_bounds(NativeInt value)
{
    if (value < 0)
        throw std::runtime_error("Negative address");
    else if (value >= main_memory_size)
        throw std::runtime_error("Overflowing address");
}

// Every negative MIX integral value must be representable by NativeInt
static_assert(-(lut[numerical_bytes_in_word] - 1) >= std::numeric_limits<NativeInt>::min());
// Every positive MIX integral value must be representable by NativeInt
static_assert(lut[numerical_bytes_in_word] - 1 <= std::numeric_limits<NativeInt>::max());

static __attribute__((always_inline))
constexpr NativeInt 
native_sign(Sign sign)
{
    return sign == s_plus ? 1 : -1; 
}

template <bool is_signed, size_t size>
NativeInt Int<is_signed, size>::native_sign() const
{
    return ::native_sign(sign());
}

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign &, Sign> Int<is_signed, size>::sign()
{
    if constexpr(is_signed)
        return sp[0].sign;
    else
        return s_plus;
}

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign const &, Sign> Int<is_signed, size>::sign() const
{
    return REMOVE_CONST_FROM_PTR(this)->sign();
}

template <bool is_signed, size_t size>
NativeInt Int<is_signed, size>::native_value() const
{
    NativeInt accum = 0;
    for (size_t i = is_signed; i < sp.size(); i++)
        accum += lut[i] * sp[i].byte;
    return native_sign() * accum;
}

template <bool is_signed, size_t size>
NativeInt IntView<is_signed, size>::native_sign() const
{
    return ::native_sign(sign());
}

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign const &, Sign> IntView<is_signed, size>::sign() const
{
    if constexpr(is_signed)
        return sp[0].sign;
    else
        return s_plus;
}

template <bool is_signed, size_t size>
NativeInt IntView<is_signed, size>::native_value() const
{
    NativeInt accum = 0;
    for (size_t i = is_signed; i < size; i++)
        accum += lut[i] * sp[i].byte;
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

template <bool is_signed, size_t size>
std::conditional_t<is_signed, Sign &, Sign> Register<is_signed, size>::sign()
{
    if constexpr (is_signed)
        return arr[0].sign;
    else
        return s_plus;
}

template <bool is_signed, size_t size>
Int<is_signed, size> Register<is_signed, size>::value()
{
    return {.sp = arr};
}

template <bool is_signed, size_t size>
IntView<is_signed, size> Register<is_signed, size>::value() const
{
    return REMOVE_CONST_FROM_PTR(this)->value();
}

template <bool is_signed, size_t size>
NativeInt Register<is_signed, size>::native_value() const
{
    return value().native_value();
}

template <bool is_signed, size_t size>
void Register<is_signed, size>::store(std::span<Byte, size> sp)
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
    else if constexpr (overflow_policy == OverflowPolicy::throw_exception)
        throw std::runtime_error("overflow after conversion to bytes");
    store(bytes);
}

__attribute__((always_inline))
void Op::operator()(Machine &m) const
{
    (m.*do_op)();
}

__attribute__((always_inline))
std::tuple<NumberRegister &, NumberRegister &> Machine::rAX()
{
    return {rA, rX};
}

__attribute__((always_inline))
Instruction Machine::current_instruction()
{
    return Instruction{Word{.sp = std::span<Byte, 6>( memory.begin() + pc, 6 )}, .m = *this};
}

__attribute__((always_inline))
void Machine::increment_pc()
{
    pc += bytes_in_word;
}

__attribute__((always_inline))
std::optional<std::reference_wrapper<IndexRegister>> Machine::get_index_register(NativeByte index)
{
    switch (index)
    {
    case 1:
        return rI1;
    case 2:
        return rI2;
    case 3:
        return rI3;
    case 4:
        return rI4;
    case 5:
        return rI5;
    case 6:
        return rI6;
    default:
        return {};
    }
}

__attribute__((always_inline))
std::span<Byte, 6> Machine::get_memory_word(NativeInt address)
{
    check_address_bounds(address);
    return std::span<Byte, 6>{memory.begin() + address * bytes_in_word, bytes_in_word};
}