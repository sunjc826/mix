#pragma once
#include <base/types.h>
#include <base/validation/validator.h>
#include <base/character_set.h>

namespace mix
{

template <bool (*validator)(NativeInt), typename ConversionT = NativeInt, typename ChildT = void>
class ValidatedInt;

template <NativeInt value>
using ValidatedLiteral = ValidatedInt<is_exact_value<value>>;
using ValidatedAddress = ValidatedInt<is_mix_address>;
using ValidatedByte = ValidatedInt<is_mix_byte, NativeByte>;
using ValidatedChar = ValidatedInt<is_mix_char, NativeByte>;
using ValidatedRegisterIndex = ValidatedInt<is_register_index, NativeByte>;
class ValidatedWord;
using ValidatedPositiveWord = ValidatedInt<is_mix_positive_word>;

}
