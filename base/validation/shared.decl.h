#pragma once
#include <base/types.h>

class ValidatedWord;

inline
std::array<Byte, bytes_in_word> 
as_bytes(ValidatedWord word);
