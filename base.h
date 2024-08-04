#pragma once
#include <utilities.h>

#include <algorithm>
#include <cmath>
#include <config.h>
#include <functional>
#include <optional>
#include <stdexcept>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <span>
#include <type_traits>
#include <tuple>

constexpr size_t main_memory_size = 4000 /* MIX words */;

// A native byte always represents 256 values
constexpr size_t native_byte_size = 256;

// A MIX byte must represent at least 64 values
constexpr size_t minimum_byte_size = 64;

// A MIX word comprises 1 sign and 5 numerical bytes
constexpr size_t bytes_in_word = 6;
constexpr size_t numerical_bytes_in_word = bytes_in_word - 1;
// extended word is stored by rAX
constexpr size_t bytes_in_extended_word = 2 * numerical_bytes_in_word + 1;

// The actual configured size of a MIX byte
constexpr size_t byte_size = MIX_BYTE_SIZE;

// The configured size of a MIX byte must be greater or equal to minimum_byte_size
static_assert(byte_size >= minimum_byte_size);

template <typename T>
constexpr size_t representable_values_v = sizeof(T) * native_byte_size;

// A native unsigned integer type capable of representing all values in a MIX byte.
// Purposely made unsigned since bytes have no sign.
using NativeByte = unsigned;
// The native size of NativeByte must be at least at large as the configured size of a MIX byte 
static_assert(representable_values_v<NativeByte> >= byte_size);

// A native signed integer type capable of representing any MIX integral value.
// Instead of doing big integer arithmetic, let us
// require that NativeInt is large enough to hold the largest value of 
// any representable integral value.
using NativeInt = long long;

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
static __attribute__((always_inline))
constexpr std::array<NativeInt, 2 * numerical_bytes_in_word + 1> 
pow_lookup_table(NativeByte base);


static __attribute__((always_inline))
constexpr NativeInt
pow(NativeByte base, size_t exponent);

template <size_t exponent>
static __attribute__((always_inline))
constexpr NativeInt
pow(NativeByte base);

static __attribute__((always_inline))
void check_address_bounds(NativeInt value);

// prefer enum over enum class
enum Sign : NativeByte
{
    s_plus = 0,
    s_minus = 1,
};

static __attribute__((always_inline))
constexpr Sign 
operator-(Sign sign)
{
    if (sign == s_plus) return s_minus;
    else return s_plus;
}

static __attribute__((always_inline))
constexpr NativeInt 
native_sign(Sign sign);

union Byte
{
    NativeByte byte;
    Sign sign;
};

struct Word
{
    std::span<Byte, bytes_in_word> sp;
    void store(std::span<Byte, bytes_in_word> new_word);
};

struct FieldSpec
{
    NativeByte L, R;

    NativeByte length() const
    {
        return R - L;
    }

    NativeByte make_F_byte() const
    {
        return 8 * L + R;
    }
};

template <bool is_signed, size_t size = std::dynamic_extent>
struct Int
{
    static constexpr size_t num_begin = is_signed ? 1 : 0;
    // If there is a sign byte, then the total number of bytes must > 1,
    // since there must be at least 1 numerical byte.
    static_assert(!is_signed || size > 1);
    std::span<Byte, size> sp;
    
    NativeInt native_sign() const;

    std::conditional_t<is_signed, Sign &, Sign> sign();

    std::conditional_t<is_signed, Sign const &, Sign> sign() const;

    NativeInt native_value() const;
};

template <bool is_signed, size_t size = std::dynamic_extent>
struct IntView
{
    static constexpr size_t num_begin = is_signed ? 1 : 0;
    // If there is a sign byte, then the total number of bytes must > 1,
    // since there must be at least 1 numerical byte.
    static_assert(!is_signed || size > 1);
    std::span<Byte const, size> sp;
    IntView(Int<is_signed, size> v)
        : sp(v.sp)
    {}

    NativeInt native_sign() const;

    std::conditional_t<is_signed, Sign const &, Sign> sign() const;

    NativeInt native_value() const;
};

struct Slice
{
    std::span<Byte> sp;
    FieldSpec spec;
    Slice(std::span<Byte> sp, FieldSpec spec)
        : sp(sp.subspan(spec.L, spec.length())), spec(spec)
    {}

    Slice(Word word, FieldSpec spec)
        : sp(word.sp.subspan(spec.L, spec.length())), spec(spec)
    {}

    Sign sign() const
    {
        if (spec.L == 0)
            return sp[0].sign;
        else 
            return s_plus;
    }

    size_t length()
    {
        return spec.length();
    }

    NativeInt native_value() const
    {
        if (spec.L == 0)
        {
            Int<true> mix_int{.sp = sp};
            return mix_int.native_value();
        }
        else
        {
            Int<false> mix_int{.sp = sp};
            return mix_int.native_value();
        }
    }
};

template <size_t size>
struct ByteConversionResult
{
    std::array<Byte, size> bytes;
    bool overflow;
};

template <size_t size>
ByteConversionResult<size> as_bytes(NativeInt value);

#include <base.impl.h>