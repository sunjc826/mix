#pragma once
#include <base.h>
#include <register.decl.h>

struct ZeroRegister final
{
    Sign sign() const;
    NativeInt native_sign() const;
    NativeInt native_value() const;
    NativeInt native_unsigned_value() const;
    void store(SliceMutable slice) const;
};

template <bool is_signed, size_t size>
struct Register 
{
    static constexpr bool is_signed_v = is_signed;
    static constexpr size_t size_v = size;
    static constexpr size_t unsigned_size_v = is_signed ? size - 1 : size;
    static constexpr size_t numerical_first_idx = is_signed ? 1 : 0;
    
    static_assert(unsigned_size_v + 1 <= bytes_in_word);

    std::array<Byte, size> reg = { {[0] = is_signed ? s_plus : 0} };

    Register() = default;
    
    Register(std::array<Byte, size> reg)
        : reg(reg)
    {}
   
    template <typename EnableIfT = std::enable_if<is_signed, RegisterWithoutSign<size - 1>>>
    EnableIfT::type unsigned_register() const;

    std::span<Byte> get_span() { return reg; }

    std::conditional_t<is_signed, Sign &, Sign> sign();
    Sign sign() const;
    NativeInt native_sign() const;

    IntMutable<is_signed, size> value();
    IntMutable<false, unsigned_size_v> unsigned_value();

    IntView<is_signed, size> value() const;
    IntView<false, unsigned_size_v> unsigned_value() const;

    NativeInt native_value() const;
    NativeInt native_unsigned_value() const;

    void load(std::span<Byte const, size> sp);

    template <typename EnableIfT = std::enable_if<is_signed, void>>
    EnableIfT::type load(Sign sign, std::span<Byte const, size - 1> sp);

    // Returns whether the load overflows
    template <bool throw_on_overflow>
    std::conditional_t<throw_on_overflow, void, bool> load(NativeInt value);

    void load_zero(Sign sign)
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

    void store(SliceMutable slice) const;

    void shift_left(NativeInt shift_by);

    void shift_right(NativeInt shift_by);

    void shift_left_circular(NativeInt shift_by);

    void shift_right_circular(NativeInt shift_by);

    operator IntegralContainer<OwnershipKind::mutable_view, is_signed, size>()
    {
        return IntegralContainer<OwnershipKind::mutable_view, is_signed, size>(reg);
    }

    operator IntegralContainer<OwnershipKind::view, is_signed, size>() const
    {
        return IntegralContainer<OwnershipKind::view, is_signed, size>(reg);
    }
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
    bool increment(NativeInt addend) 
    { 
        return load<false>(native_value() + addend); 
    };
};

struct IndexRegister final : public Register<true, 3>
{
    bool increment(NativeInt addend)
    {
        load<true>(native_value() + addend);
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
