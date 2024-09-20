#pragma once
#include <base/types.h>
#include <base/validator.decl.h>

template <typename StorageT, bool (*validator)(NativeInt), typename ChildT = void>
class ValidatedInt;

template <NativeInt value>
using ValidatedLiteral = ValidatedInt<NativeInt, is_exact_value<value>>;
using ValidatedAddress = ValidatedInt<NativeInt, is_mix_address>;
using ValidatedByte = ValidatedInt<NativeByte, is_mix_byte>;
using ValidatedRegisterIndex = ValidatedInt<NativeByte, is_register_index>;
class ValidatedWord;
using ValidatedPositiveWord = ValidatedInt<NativeInt, is_mix_positive_word>;

inline
std::array<Byte, bytes_in_word> as_bytes(ValidatedWord word);
