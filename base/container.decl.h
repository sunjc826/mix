#pragma once
#include "base/types.decl.h"
#include "base/validation/validator.impl.h"
#include "config.impl.h"
#include <base/types.h>
#include <base/validation/types.h>
#include <utility>

namespace mix
{


template <bool is_view, typename T>
struct ConstIfView
{
    using type = T;
};

template <typename T>
struct ConstIfView<true, T>
{
    using type = T const;
};

template <bool is_view, typename T>
using ConstIfView_t = typename ConstIfView<is_view, T>::type;

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

struct FieldSpec
{
    NativeByte L, R;

    static FieldSpec from_byte(NativeByte field)
    {
        return {.L = field / 8, .R = field % 8};
    }

    NativeByte length() const
    {
        return R - L;
    }

    NativeByte make_F_byte() const
    {
        return 8 * L + R;
    }
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
        if constexpr (size <= bytes_in_extended_word)
        {
            // Here, copy elision is possible even for a Validated type.
            // However note that `constructor_helper` results in a fully "unrolled" `size` assignments,
            // last I checked on godbolt.
            return constructor_helper(sp, std::make_index_sequence<size>());
        }
        else
        {
            // This is not allowed for a Validated type as it is not trivially constructible.
            // We would have to resort to DeferredValue (or even std::optional) which leads to additional copying.
            std::array<T, bytes_in_word> arr;
            std::copy(sp.begin(), sp.end(), arr.begin());
            return arr;
        }
    }

private:
    template <size_t ...Is>
    static type constructor_helper(std::span<element_type, size> sp, std::index_sequence<Is...>)
    {
        return { sp[Is]... };
    }
};

template <typename T, size_t size>
struct OwnershipKindContainer<OwnershipKind::mutable_view, T, size>
{
    using element_type = T;
    using type = std::span<element_type, size>;
    static constexpr bool is_view = false;
    [[gnu::always_inline]]
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
    [[gnu::always_inline]]
    static type constructor(std::span<element_type, size> sp)
    {
        return sp;
    }
};

template <OwnershipKind kind, bool is_signed, size_t size>
struct IntegralContainer
{
    using Container = OwnershipKindContainer<kind, Byte, size>;
    static constexpr bool is_view = Container::is_view;
    typename Container::type container;
    // If there is a sign byte, then the total number of bytes must > 1,
    // since there must be at least 1 numerical byte.
    static constexpr size_t num_begin = is_signed ? 1 : 0;
    static_assert(size > 0);
    static constexpr size_t unsigned_size = is_signed ? size - 1 : size;
    static_assert(unsigned_size > 0);

    IntegralContainer()
        : container()
    {}

    IntegralContainer(std::span<typename Container::element_type, size> sp)
        : container(Container::constructor(sp))
    {}

    IntegralContainer(std::array<typename Container::element_type, size> arr)
        : container(Container::constructor(std::span<typename Container::element_type, size>(arr.begin(), arr.size())))
    {}

    template <typename T = void>
    requires (kind == OwnershipKind::view || kind == OwnershipKind::mutable_view)
    IntegralContainer(IntegralContainer<OwnershipKind::owns, is_signed, size> &w)
        : container(Container::constructor(w.container))
    {}

    template <typename T = void>
    requires (kind == OwnershipKind::view)
    IntegralContainer(IntegralContainer<OwnershipKind::mutable_view, is_signed, size> const &w)
        : container(Container::constructor(w.container))
    {}

    IntegralContainer(NativeInt value)
        : container()
    {
        if constexpr (is_signed)
        {
            if (value < 0)
            {
                container[0].sign = s_minus;
                value = -value;
            }
            else
            {
                container[0].sign = s_plus;
            }
        }
        else
        {
            if (value < 0)
                throw std::runtime_error("Negative value cannot be placed in an unsigned MIX integer");
        }

        for (size_t s = size; s --> 1;)
        {
            container[s].byte = value % byte_size;
            value /= byte_size;
        }

        if (value > 0)
            throw std::runtime_error("Overflow");
    }

    ValidatedInt<IsInClosedInterval<-1, 1>> native_sign() const;

    template <typename SelfT = IntegralContainer>
    requires (!SelfT::is_view)
    std::conditional_t<is_signed, Sign &, Sign> sign();

    std::conditional_t<is_signed, Sign const &, Sign> sign() const;

    struct TypeHolder
    {
        using type = ValidatedInt<IsInClosedInterval<-(lut[unsigned_size] - 1), lut[unsigned_size] - 1>>;
    };
    std::conditional_t<
        size == std::dynamic_extent,
        std::type_identity<Result<ValidatedInt<IsMixWord>>>,
        TypeHolder
    >::type native_value() const;
};

template <OwnershipKind kind>
struct Word : IntegralContainer<kind, true, bytes_in_word>
{
    using Container = OwnershipKindContainer<kind, Byte, bytes_in_word>; 
    using Ref = std::conditional_t<kind == OwnershipKind::owns, Word<kind> &, Word<kind> const &>;   
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

    Slice(Slice const &slice) = default;

    template <typename = void>
    requires (is_view)
    Slice(Slice<false> const &slice)
        : sp(slice.sp), spec(slice.spec)
    {}

    template <size_t size>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::owns, true, size> &i)
        : sp(std::span<PossiblyConstByte, size>(i.container)), spec{.L = 0, .R = static_cast<NativeByte>(sp.size())}
    {
    }

    template <size_t size>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::mutable_view, true, size> const &i)
        : sp(std::span<PossiblyConstByte, size>(i.container)), spec{.L = 0, .R = static_cast<NativeByte>(sp.size())}
    {
    }

    template <size_t size>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::owns, true, size> &word, FieldSpec spec)
        : sp(std::span<PossiblyConstByte, bytes_in_word>(word.container).subspan(spec.L, spec.length())), spec(spec)
    {
    }

    template <size_t size>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::mutable_view, true, size> const &i, FieldSpec spec)
        : sp(std::span<PossiblyConstByte, size>(i.container).subspan(spec.L, spec.length())), spec(spec)
    {
    }

    template <size_t size, typename EnableIfT = std::enable_if<!is_view>>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::view, true, size> const &word, FieldSpec spec, EnableIfT * = 0)
        : sp(std::span<PossiblyConstByte, size>(word.container).subspan(spec.L, spec.length())), spec(spec)
    {
    }

    template <size_t size, typename EnableIfT = std::enable_if<!is_view>>
    requires (size < 8)
    Slice(IntegralContainer<OwnershipKind::view, true, size> const &i, EnableIfT * = 0)
        : sp(std::span<PossiblyConstByte, size>(i.container)), spec{.L = 0, .R = static_cast<NativeByte>(sp.size())}
    {
    }

   
    bool is_signed() const
    {
        return spec.L == 0;
    }

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

}
