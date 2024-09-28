#pragma once
#include <base/validation/v2.h>
namespace mix
{

constexpr ValidatedLiteral<0> zero = ValidatedLiteral<0>::constructor(0).value();
constexpr ValidatedLiteral<1> one = ValidatedLiteral<1>::constructor(1).value();
constexpr ValidatedLiteral<2> two = ValidatedLiteral<2>::constructor(2).value();
constexpr ValidatedLiteral<3> three = ValidatedLiteral<3>::constructor(3).value();
constexpr ValidatedLiteral<4> four = ValidatedLiteral<4>::constructor(4).value();
constexpr ValidatedLiteral<5> five = ValidatedLiteral<5>::constructor(5).value();
constexpr ValidatedLiteral<6> six = ValidatedLiteral<6>::constructor(6).value();
constexpr ValidatedLiteral<byte_size> validated_byte_size = ValidatedLiteral<byte_size>::constructor(byte_size).value();

}
