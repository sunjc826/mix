#include <register.h>

void ExtendedRegister::store(NativeInt value)
{
    ByteConversionResult<bytes_in_extended_word> const result = as_bytes<bytes_in_extended_word>(value);
    if (result.overflow)
        throw std::runtime_error("Unexpected overflow during multiplication");
    Sign const sign = result.bytes[0].sign;
    rA.store(sign, std::span<Byte const, numerical_bytes_in_word>(result.bytes.begin() + 1, numerical_bytes_in_word));
    rX.store(sign, std::span<Byte const, numerical_bytes_in_word>(result.bytes.begin() + bytes_in_word + 1, numerical_bytes_in_word));
}

NativeInt ExtendedRegister::native_value() const
{
    NativeInt const unsigned_rAX_value = rA.native_unsigned_value() * lut[numerical_bytes_in_word] + rX.native_unsigned_value();
    NativeInt const sign = rA.native_sign();
    return sign * unsigned_rAX_value;
}
