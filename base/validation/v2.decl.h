#pragma once
#include "check.h"
#include <base/types.h>
#include <base/validation/validator.h>
#include <base/character_set.h>

namespace mix
{

template <typename StorageT, typename ValidatorT, typename ConversionT = StorageT, typename ChildT = void>
class ValidatedObject;

template <typename ValidatorT, typename ConversionT = NativeInt, typename ChildT = void>
class ValidatedInt;

template <NativeInt value>
using ValidatedLiteral = ValidatedInt<IsExactValue<value>>;
using ValidatedPositive = ValidatedInt<IsPositive>;
using ValidatedNonNegative = ValidatedInt<IsNonNegative>;
template <NativeInt low, NativeInt high>
using ValidatedBounded = ValidatedInt<IsInClosedInterval<low, high>>;
using ValidatedAddress = ValidatedInt<IsMixAddress>;
using ValidatedByte = ValidatedInt<IsMixByte, NativeByte>;
using ValidatedChar = ValidatedInt<IsMixChar, NativeByte>;
using ValidatedRegisterIndex = ValidatedInt<IsRegisterIndex, NativeByte>;
using ValidatedWord = ValidatedInt<IsMixWord>;
using ValidatedPositiveWord = ValidatedInt<IsMixPositiveWord>;



}
