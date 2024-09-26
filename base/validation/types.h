#pragma once
#include <base/types.h>
#include <base/deferred_value.h>
#include <base/validation/v2.h>
namespace mix
{
union Byte
{
    ValidatedByte byte;
    Sign sign;
};

template <size_t size>
struct ByteConversionResult
{
    std::array<Byte, size> bytes;
    bool overflow;
};

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

inline
std::array<Byte, bytes_in_word> 
as_bytes(ValidatedWord word)
{
    auto const [
        sign, 
        abs_value
    ] = ValidatedConstructors::from_abs(word);
    ValidatedNonNegative value = abs_value;

    std::array<DeferredValue<Byte>, bytes_in_word> result;
    for (size_t s = result.size(); s --> 1;)
    {
        ValidatedBounded<0, byte_size - 1> const residue = ValidatedConstructors::from_mod<byte_size>(value);
        result[s].construct(residue);
        value = value.divide<byte_size>();
    }

    result[0].construct(sign);

    // As long as we use DeferredValue, 
    // we will need to give up copy elision,
    // which is unfortunate.
    return actualize_reinterpret(result);
}


}
