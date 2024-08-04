#pragma once
#include <base.h>
#include <register.decl.h>

template <bool is_signed, size_t size>
struct Register
{
    static constexpr bool is_signed_v = is_signed;
    static constexpr size_t size_v = size;
    static constexpr size_t unsigned_size_v = is_signed ? size - 1 : size;
    static constexpr size_t numerical_first_idx = is_signed ? 1 : 0;

    std::array<Byte, size> arr;

    std::conditional_t<is_signed, Sign &, Sign> sign();
    Sign sign() const;
    NativeInt native_sign() const;

    Int<is_signed, size> value();
    Int<false, unsigned_size_v> unsigned_value();

    IntView<is_signed, size> value() const;
    IntView<false, unsigned_size_v> unsigned_value() const;

    NativeInt native_value() const;
    NativeInt native_unsigned_value() const;

    void store(std::span<Byte const, size> sp);

    template <typename EnableIfT = std::enable_if<is_signed, void>>
    EnableIfT::type store(Sign sign, std::span<Byte const, size - 1> sp);

    // Returns whether the store overflows
    template <bool throw_on_overflow = false>
    bool store(NativeInt value);

    void shift_left(NativeInt shift_by);

    void shift_right(NativeInt shift_by);

    void shift_left_circular(NativeInt shift_by);

    void shift_right_circular(NativeInt shift_by);
};

struct NumberRegister : public Register<true, 6>
{
};

struct IndexRegister : public Register<true, 3>
{
};

struct JumpRegister : public Register<false, 2>
{
};

struct ExtendedRegister
{
    NumberRegister &rA, &rX;

    Sign sign() const;
    NativeInt native_value() const;

    void store(NativeInt value);

    void shift_left(NativeInt shift_by);

    void shift_right(NativeInt shift_by);

    void shift_left_circular(NativeInt shift_by);

    void shift_right_circular(NativeInt shift_by);
};

#include <register.impl.h>
