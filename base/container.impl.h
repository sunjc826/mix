#pragma once
#include <base/container.decl.h>
#include <base/math.h>
namespace mix
{
template <OwnershipKind kind, bool is_signed, size_t size>
ValidatedInt<IsInClosedInterval<-1, 1>> IntegralContainer<kind, is_signed, size>::native_sign() const
{
    return ValidatedConstructors::from_sign(sign());
}

template <OwnershipKind kind, bool is_signed, size_t size>
template <typename>
requires (!IntegralContainer<kind, is_signed, size>::is_view)
std::conditional_t<is_signed, Sign &, Sign> IntegralContainer<kind, is_signed, size>::sign()
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
 std::conditional_t<
    size == std::dynamic_extent,
    std::type_identity<NativeInt>,
    typename IntegralContainer<kind, is_signed, size>::TypeHolder
>::type IntegralContainer<kind, is_signed, size>::native_value() const
{
    NativeInt accum = 0;
    for (size_t i = is_signed; i < container.size(); i++)
        accum += lut[i] * container[i].byte;
    return native_sign() * accum;
}

}
