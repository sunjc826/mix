#include "register.decl.h"
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

Sign ExtendedRegister::sign() const
{
    return rA.sign();
}

NativeInt ExtendedRegister::native_value() const
{
    NativeInt const unsigned_rAX_value = rA.native_unsigned_value() * lut[numerical_bytes_in_word] + rX.native_unsigned_value();
    NativeInt const sign = rA.native_sign();
    return sign * unsigned_rAX_value;
}

void ExtendedRegister::shift_left(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Shift should be non-negative");
    Register<false, NumberRegister::unsigned_size_v * 2> reg;
    reg.shift_left(shift_by);
    for (size_t i = 0; i < NumberRegister::unsigned_size_v; i++)
    {
        rA.arr[i + 1].byte = reg.arr[i].byte;
        rX.arr[i + 1].byte = reg.arr[i + NumberRegister::unsigned_size_v].byte;
    }
}

void ExtendedRegister::shift_right(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Shift should be non-negative");
    Register<false, NumberRegister::unsigned_size_v * 2> reg;
    reg.shift_right(shift_by);
    for (size_t i = 0; i < NumberRegister::unsigned_size_v; i++)
    {
        rA.arr[i + 1].byte = reg.arr[i].byte;
        rX.arr[i + 1].byte = reg.arr[i + NumberRegister::unsigned_size_v].byte;
    }
}

void ExtendedRegister::shift_left_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Shift should be non-negative");
    Register<false, NumberRegister::unsigned_size_v * 2> reg;
    reg.shift_left_circular(shift_by);
    for (size_t i = 0; i < NumberRegister::unsigned_size_v; i++)
    {
        rA.arr[i + 1].byte = reg.arr[i].byte;
        rX.arr[i + 1].byte = reg.arr[i + NumberRegister::unsigned_size_v].byte;
    }
}

void ExtendedRegister::shift_right_circular(NativeInt shift_by)
{
    if (shift_by < 0)
        throw std::runtime_error("Shift should be non-negative");
    Register<false, NumberRegister::unsigned_size_v * 2> reg;
    reg.shift_right_circular(shift_by);
    for (size_t i = 0; i < NumberRegister::unsigned_size_v; i++)
    {
        rA.arr[i + 1].byte = reg.arr[i].byte;
        rX.arr[i + 1].byte = reg.arr[i + NumberRegister::unsigned_size_v].byte;
    }
}
