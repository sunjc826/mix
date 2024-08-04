#pragma once
#include <base.h>
#include <register.decl.h>

struct TypeErasedRegister
{
    virtual bool get_is_signed() const = 0;
    virtual size_t get_size() const = 0;
    virtual size_t get_unsigned_size() const = 0;
    virtual size_t get_numerical_first_idx() const = 0;
    virtual bool load_no_throw_on_overflow(NativeInt value) = 0;
    virtual void load_throw_on_overflow(NativeInt value) = 0;
    void load(Slice slice) { load_throw_on_overflow(slice.native_value()); }
    virtual void store(Slice slice);
};

template <bool is_signed, size_t size>
struct Register : TypeErasedRegister
{
    static constexpr bool is_signed_v = is_signed;
    static constexpr size_t size_v = size;
    static constexpr size_t unsigned_size_v = is_signed ? size - 1 : size;
    static constexpr size_t numerical_first_idx = is_signed ? 1 : 0;

    bool get_is_signed() const override final { return is_signed_v; }
    size_t get_size() const override final { return size_v; }
    size_t get_unsigned_size() const override final { return unsigned_size_v; }
    size_t get_numerical_first_idx() const override final { return numerical_first_idx; }

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

    void load(std::span<Byte const, size> sp);

    template <typename EnableIfT = std::enable_if<is_signed, void>>
    EnableIfT::type load(Sign sign, std::span<Byte const, size - 1> sp);

    // Returns whether the load overflows
    template <bool throw_on_overflow = false>
    bool load(NativeInt value);

    bool load_no_throw_on_overflow(NativeInt value) override final { return load<false>(value); }
    void load_throw_on_overflow(NativeInt value) override final { load<true>(value); };
    
    void store(Slice slice) override final;

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

    void load(NativeInt value);

    void shift_left(NativeInt shift_by);

    void shift_right(NativeInt shift_by);

    void shift_left_circular(NativeInt shift_by);

    void shift_right_circular(NativeInt shift_by);
};

#include <register.impl.h>
