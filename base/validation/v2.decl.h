#pragma once
#include <base/types.h>
#include <base/validator.h>
#include <base/character_set.h>

namespace mix
{

template <typename StorageT, typename ValidatorT, typename ConversionT, typename ChildT = void>
class ValidatedObject;

template <typename ValidatorT, typename ConversionT = NativeInt, typename ChildT = void>
class ValidatedInt;

template <NativeInt value>
using ValidatedLiteral = ValidatedInt<IsExactValue<value>>;
using ValidatedAddress = ValidatedInt<IsMixAddress>;
using ValidatedByte = ValidatedInt<IsMixByte, NativeByte>;
using ValidatedChar = ValidatedInt<IsMixChar, NativeByte>;
using ValidatedRegisterIndex = ValidatedInt<IsRegisterIndex, NativeByte>;
class ValidatedWord;
using ValidatedPositiveWord = ValidatedInt<IsMixPositiveWord, NativeInt>;



}
