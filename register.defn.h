#pragma once
#include <base.h>
#include <register.decl.h>

// Useful if we get overwhelmed by templates and want to use runtime polymorphism
struct TypeErasedRegister
{
    virtual bool get_is_signed() const = 0;
    virtual size_t get_size() const = 0;
    virtual size_t get_unsigned_size() const = 0;
    virtual size_t get_numerical_first_idx() const = 0;
    virtual std::span<Byte> get_span() = 0;
    virtual NativeInt native_value() const = 0;
    virtual NativeInt native_unsigned_value() const = 0;
    virtual bool load_no_throw_on_overflow(NativeInt value) = 0;
    virtual void load_throw_on_overflow(NativeInt value) = 0;
    virtual void load_zero(Sign sign) = 0;
    virtual bool increment(NativeInt addend) { return load_no_throw_on_overflow(native_value() + addend); };
    void load(SliceView slice) { load_throw_on_overflow(slice.native_value()); }
    virtual void store(SliceMutable slice) const = 0;
    SliceMutable make_slice(FieldSpec spec);
};

struct ZeroRegister final
{
    Sign sign() const;
    NativeInt native_sign() const;
    NativeInt native_value() const;
    NativeInt native_unsigned_value() const;
    void store(SliceMutable slice) const;
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

    std::array<Byte, size> reg = {};

    Register() = default;
    
    Register(std::array<Byte, size> reg)
        : reg(reg)
    {}
   
    template <typename EnableIfT = std::enable_if<is_signed, RegisterWithoutSign<size - 1>>>
    EnableIfT::type unsigned_register() const;

    std::span<Byte> get_span() override final { return reg; }

    std::conditional_t<is_signed, Sign &, Sign> sign();
    Sign sign() const;
    NativeInt native_sign() const;

    IntMutable<is_signed, size> value();
    IntMutable<false, unsigned_size_v> unsigned_value();

    IntView<is_signed, size> value() const;
    IntView<false, unsigned_size_v> unsigned_value() const;

    NativeInt native_value() const override final;
    NativeInt native_unsigned_value() const override final;

    void load(std::span<Byte const, size> sp);

    template <typename EnableIfT = std::enable_if<is_signed, void>>
    EnableIfT::type load(Sign sign, std::span<Byte const, size - 1> sp);

    // Returns whether the load overflows
    template <bool throw_on_overflow = false>
    bool load(NativeInt value);

    bool load_no_throw_on_overflow(NativeInt value) override final { return load<false>(value); }
    void load_throw_on_overflow(NativeInt value) override final { load<true>(value); };

    void load_zero(Sign sign) override final 
    { 
        if constexpr (is_signed)
        {
            reg[0].sign = sign; 
            std::fill(reg.begin() + 1, reg.end(), Byte{.byte = 0});
        }
        else
        {
            std::fill(reg.begin(), reg.end(), Byte{.byte = 0});
        }
    }

    void store(SliceMutable slice) const override final;

    void shift_left(NativeInt shift_by);

    void shift_right(NativeInt shift_by);

    void shift_left_circular(NativeInt shift_by);

    void shift_right_circular(NativeInt shift_by);
};

template <size_t size>
struct RegisterWithoutSign final
{
    std::span<Byte, size> reg;

    RegisterWithoutSign(Register<true, size> register_storage)
        : reg(register_storage.reg.begin(), register_storage.reg.end())
    {}

    IntMutable<false, size> unsigned_value();
    IntView<false, size> unsigned_value() const;

    NativeInt native_unsigned_value() const;

    void store(Sign sign, SliceMutable slice) const;
};

struct NumberRegister final : public Register<true, 6>
{ 
};

struct IndexRegister final : public Register<true, 3>
{
    bool increment(NativeInt addend) override final
    {
        load_throw_on_overflow(native_value() + addend);
        return false;
    }
};

struct JumpRegister final : public Register<false, 2>
{
};

struct ExtendedRegister final
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
