#pragma once
#include <base/container.decl.h>
#include <base/math.h>
#include <utility>
namespace mix
{
template <OwnershipKind kind, bool is_signed, size_t size>
ValidatedInt<IsInClosedInterval<-1, 1>> IntegralContainer<kind, is_signed, size>::native_sign() const
{
    return ValidatedUtils::from_sign(sign());
}

template <OwnershipKind kind, bool is_signed, size_t size>
template <typename SelfT>
requires (!SelfT::is_view)
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

namespace details {
template <size_t offset, typename ContainerT, size_t ...Is>
ValidatedInt<IsInClosedInterval<0, lut[sizeof...(Is)] - 1>>
multiply_add(ContainerT const &container, std::index_sequence<Is...>)
{
    return ((to_interval(from_literal<lut[sizeof...(Is) - 1 - Is]>()) * to_interval(container[offset + Is].byte)) + ...);
}
}

template <OwnershipKind kind, bool is_signed, size_t size>
 std::conditional_t<
    size == std::dynamic_extent,
    std::type_identity<Result<ValidatedInt<IsMixWord>>>,
    typename IntegralContainer<kind, is_signed, size>::TypeHolder
>::type IntegralContainer<kind, is_signed, size>::native_value() const
{
    
    if constexpr (size == std::dynamic_extent)
    {
        NativeInt accum = 0;
        for (size_t i = is_signed; i < container.size(); i++)
            accum += lut[i] * container[i].byte;
        return ValidatedWord::constructor(native_sign() * accum);
    }
    else
    {   ValidatedInt<IsInClosedInterval<0, lut[unsigned_size] - 1>> const unsigned_result = details::multiply_add<size_t(is_signed)>(container, std::make_index_sequence<unsigned_size>());
        if constexpr (!is_signed)
            return unsigned_result;
        else
            return native_sign() * unsigned_result;
    }
}

}
