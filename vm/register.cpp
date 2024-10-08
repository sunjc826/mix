#include "base/validation/validator.impl.h"
#include <base/base.h>
#include <vm/register.h>
namespace mix
{

Sign ZeroRegister::sign() const
{
    return s_plus;
}

ValidatedInt<IsInClosedInterval<-1, 1>> ZeroRegister::native_sign() const
{
    return ValidatedUtils::from_sign(sign());
}

NativeInt ZeroRegister::native_value() const
{
    return 0;
}

NativeInt ZeroRegister::native_unsigned_value() const
{
    return 0;
}

void ZeroRegister::store(SliceMutable slice) const
{
    std::fill(
        slice.sp.begin(), slice.sp.end(), 
        deduce<IsMixByte>(zero)
    );
}

void ExtendedRegister::load(NativeInt value)
{
    ByteConversionResult<bytes_in_extended_word> const result = as_bytes<bytes_in_extended_word>(value);
    if (result.overflow)
        throw std::runtime_error("Unexpected overflow during multiplication");
    Sign const sign = result.bytes[0].sign;
    rA.load(sign, std::span<Byte const, numerical_bytes_in_word>(result.bytes.begin() + 1, numerical_bytes_in_word));
    rX.load(sign, std::span<Byte const, numerical_bytes_in_word>(result.bytes.begin() + bytes_in_word + 1, numerical_bytes_in_word));
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
        rA.reg[i + 1].byte = reg.reg[i].byte;
        rX.reg[i + 1].byte = reg.reg[i + NumberRegister::unsigned_size_v].byte;
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
        rA.reg[i + 1].byte = reg.reg[i].byte;
        rX.reg[i + 1].byte = reg.reg[i + NumberRegister::unsigned_size_v].byte;
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
        rA.reg[i + 1].byte = reg.reg[i].byte;
        rX.reg[i + 1].byte = reg.reg[i + NumberRegister::unsigned_size_v].byte;
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
        rA.reg[i + 1].byte = reg.reg[i].byte;
        rX.reg[i + 1].byte = reg.reg[i + NumberRegister::unsigned_size_v].byte;
    }
}

}
