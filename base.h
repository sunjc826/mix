#pragma once
#include <config.h>
#include <utilities.h>

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

static inline __attribute__((always_inline))
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

enum class OwnershipKind
{
    owns,
    mutable_view,
    view,
};

template <OwnershipKind kind, typename T = Byte, size_t size = kind == OwnershipKind::owns ? bytes_in_word : std::dynamic_extent>
struct OwnershipKindContainer;

template <typename T, size_t size>
struct OwnershipKindContainer<OwnershipKind::owns, T, size>
{
    using element_type = T;
    using type = std::array<element_type, size>;
    static constexpr bool is_view = false;
    static type constructor(std::span<element_type, size> sp)
    {
        std::array<Byte, bytes_in_word> arr;
        std::copy(sp.begin(), sp.end(), arr.begin());
        return arr;
    }
};

template <typename T, size_t size>
struct OwnershipKindContainer<OwnershipKind::mutable_view, T, size>
{
    using element_type = T;
    using type = std::span<element_type, size>;
    static constexpr bool is_view = false;
    __attribute__((always_inline))
    static type constructor(std::span<element_type, size> sp)
    {
        return sp;
    }
};

template <typename T, size_t size>
struct OwnershipKindContainer<OwnershipKind::view, T, size>
{
    using element_type = T const;
    using type = std::span<element_type, size>;
    static constexpr bool is_view = true;
    __attribute__((always_inline))
    static type constructor(std::span<element_type, size> sp)
    {
        return sp;
    }
};

template <OwnershipKind kind, bool is_signed, size_t size>
struct IntegralContainer
{
    using Container = OwnershipKindContainer<kind, Byte, bytes_in_word>;
    typename Container::type container;
    // If there is a sign byte, then the total number of bytes must > 1,
    // since there must be at least 1 numerical byte.
    static constexpr size_t num_begin = is_signed ? 1 : 0;
    static_assert(!is_signed || size > 1);

    IntegralContainer(std::span<typename Container::element_type, size> sp)
        : container(Container::constructor(sp))
    {}

    IntegralContainer(std::array<typename Container::element_type, size> arr)
        : container(Container::constructor(std::span<typename Container::element_type, size>(arr.begin(), arr.size())))
    {}

    template <typename EnableIfT = std::enable_if<kind == OwnershipKind::view || kind == OwnershipKind::mutable_view>>
    IntegralContainer(IntegralContainer<OwnershipKind::owns, is_signed, size> &w, EnableIfT::type * = 0)
        : container(Container::constructor(w.container))
    {}

    template <typename EnableIfT = std::enable_if<kind == OwnershipKind::view>>
    IntegralContainer(IntegralContainer<OwnershipKind::mutable_view, is_signed, size> const &w, EnableIfT::type * = 0)
        : container(Container::constructor(w.container))
    {}

    NativeInt native_sign() const;

    template <typename EnableIfT = std::enable_if<!Container::is_view, std::conditional_t<is_signed, Sign &, Sign>>>
    EnableIfT::type sign();

    std::conditional_t<is_signed, Sign const &, Sign> sign() const;

    NativeInt native_value() const;
};

template <OwnershipKind kind>
struct Word : IntegralContainer<kind, true, bytes_in_word>
{
    using Container = OwnershipKindContainer<kind, Byte, bytes_in_word>;    
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
using IntMutable = IntegralContainer<OwnershipKind::mutable_view, is_signed, size>;

template <bool is_signed, size_t size = std::dynamic_extent>
using IntView = IntegralContainer<OwnershipKind::view, is_signed, size>;

template <bool is_view>
struct Slice
{
    using PossiblyConstByte = ConstIfView_t<is_view, Byte>;
    std::span<PossiblyConstByte> sp;
    FieldSpec spec;

    Slice(Slice<false> const &slice)
        : sp(slice.sp), spec(slice.spec)
    {}

    template <size_t size>
    explicit Slice(std::span<PossiblyConstByte, size> sp)
        : sp(sp), spec{.L = 0, .R = static_cast<NativeByte>(sp.size())}
    {}

    template <size_t size>
    explicit Slice(std::span<PossiblyConstByte, size> sp, FieldSpec spec)
        : sp(sp.subspan(spec.L, spec.length())), spec(spec)
    {}

    template <size_t size, typename EnableIfT = std::enable_if<is_view, Byte>>
    explicit Slice(std::span<typename EnableIfT::type, size> sp, FieldSpec spec)
        : sp(sp.subspan(spec.L, spec.length())), spec(spec)
    {}

    Sign sign() const
    {
        if (spec.L == 0)
            return sp[0].sign;
        else 
            return s_plus;
    }

    size_t length() const
    {
        return spec.length();
    }

    NativeInt native_value() const
    {
        if (spec.L == 0)
        {
            IntView<true> mix_int(sp);
            return mix_int.native_value();
        }
        else
        {
            IntView<false> mix_int(sp);
            return mix_int.native_value();
        }
    }
};

using SliceMutable = Slice<false>;

using SliceView = Slice<true>;

template <size_t size>
struct ByteConversionResult
{
    std::array<Byte, size> bytes;
    bool overflow;
};

template <size_t size>
ByteConversionResult<size> as_bytes(NativeInt value);

#include <base.impl.h>