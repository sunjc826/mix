#pragma once
#include <config.h>
#include <error.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>

#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <span>
#include <type_traits>
#include <tuple>

using namespace std::string_view_literals;
namespace mix
{
// The size of main memory of a MIX machine in terms of the number of MIX words
constexpr size_t main_memory_size = 4000;

// A native byte always represents 256 values
constexpr size_t native_byte_size = 256;

// A MIX byte must represent at least 64 values
constexpr size_t minimum_byte_size = 64;

// A MIX word comprises 1 sign and 5 numerical bytes
constexpr size_t bytes_in_word = 6;

constexpr size_t numerical_bytes_in_word = bytes_in_word - 1;

// extended word is stored by rAX
constexpr size_t bytes_in_extended_word = 2 * numerical_bytes_in_word + 1;

constexpr size_t numerical_bytes_in_extended_word = bytes_in_extended_word - 1;

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

using Validator = bool (*)(NativeInt);

template <bool (*validator)(NativeInt)>
using ValidatorConstant = std::integral_constant<bool (*)(NativeInt), validator>;

static inline __attribute__((always_inline))
void 
check_address_bounds(NativeInt value);

// prefer enum over enum class
enum Sign : NativeByte
{
    s_plus = 0,
    s_minus = 1,
};

static __attribute__((always_inline))
constexpr 
Sign 
operator-(Sign sign)
{
    if (sign == s_plus) return s_minus;
    else return s_plus;
}

static __attribute__((always_inline))
constexpr
NativeInt
native_sign(Sign sign);

#define REMOVE_CONST_FROM_PTR(ptr)\
    const_cast<std::remove_const_t<std::remove_reference_t<decltype(*ptr)>> *>(ptr)


}
