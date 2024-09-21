#pragma once
#include <cstddef>
struct TypeErasedRegister;
template <size_t size>
struct RegisterWithoutSign;
template <bool is_signed, size_t size>
struct Register;

struct NumberRegister;
struct IndexRegister;
struct JumpRegister;

struct ExtendedRegister;