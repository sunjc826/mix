#pragma once
#include <base/types.h>
#include <base/deferred_value.h>
#include <base/validation/v2.h>
#include <base/validation/constants.h>
namespace mix
{
union Byte
{
    ValidatedByte byte;
    Sign sign;
    constexpr
    Byte();
    [[gnu::always_inline]]
    constexpr
    Byte(ValidatedByte byte) : byte(byte) {}
    [[gnu::always_inline]]
    constexpr
    Byte(Sign sign) : sign(sign) {}
    Byte &
    operator=(ValidatedByte byte) { this->byte = byte; return *this; }
    Byte &
    operator=(Sign sign) { this->sign = sign; return *this; }
};

constexpr Byte zero_byte(deduce_sequence<TypeSequence<IsInClosedInterval<0, byte_size - 1>, IsMixByte>>(to_interval(zero)));

inline 
constexpr
Byte::Byte()
    : Byte(zero_byte)
{}

template <size_t size>
struct ByteConversionResult
{
    std::array<Byte, size> bytes;
    bool overflow;
};

template <size_t size>
ByteConversionResult<size> 
as_bytes(NativeInt i)
{
    static constexpr ValidatedPositive validated_byte_size = deduce_sequence<TypeSequence<IsInClosedInterval<1, byte_size>, IsPositive>>(from_literal<byte_size>());

    auto const [
        sign, 
        abs_value
    ] = ValidatedUtils::from_abs(i);
    ValidatedNonNegative value = abs_value;

    std::array<DeferredValue<Byte>, size> result;
    for (size_t s = size; s --> 1;)
    {
        ValidatedBounded<0, byte_size - 1> const residue = ValidatedUtils::from_mod<byte_size>(value);
        result[s].construct(residue);
        value = value / validated_byte_size;
    }

    result[0].construct(sign);
    
    return ByteConversionResult<size>{
        .bytes = actualize_reinterpret(result),
        .overflow = value > 0
    };
}

inline
std::array<Byte, bytes_in_word> 
as_bytes(ValidatedWord word)
{
    static constexpr auto validated_ten = deduce_sequence<TypeSequence<IsInClosedInterval<0, byte_size - 1>, IsNonNegative>>(from_literal<10>());

    auto const [
        sign, 
        abs_value
    ] = ValidatedUtils::from_abs(word);
    ValidatedNonNegative value = abs_value;

    std::array<DeferredValue<Byte>, bytes_in_word> result;
    for (size_t s = result.size(); s --> 1;)
    {
        ValidatedBounded<0, byte_size - 1> const residue = ValidatedUtils::from_mod<byte_size>(value);
        result[s].construct(residue);
        value = value / validated_ten;
    }

    result[0].construct(sign);

    // As long as we use DeferredValue, 
    // we will need to give up copy elision,
    // which is unfortunate.
    return actualize_reinterpret(result);
}


}
