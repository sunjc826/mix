#include "base/validation/v2.decl.h"
#include "base/validation/v2.defn.h"
#include "base/validation/validator.impl.h"
#include <base/base.h>
#include <vm/instruction.h>
#include <vm/machine.h>
#include <vm/register.decl.h>

#include <compare>
#include <iostream>
#include <stdexcept>
namespace mix
{

void Machine::do_nop()
{
    increment_pc();
}

void Machine::do_add()
{
    if (rA.load<false>(rA.native_value() + inst.native_MF()))
        overflow = true;
    increment_pc();
}

void Machine::do_fadd()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_sub()
{
    if (rA.load<false>(rA.native_value() - inst.native_MF()))
        overflow = true;
    increment_pc();
}

void Machine::do_fsub()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_mul()
{
    NativeInt const mul_result = rA.native_value() * inst.native_MF();
    rAX.load(mul_result);
    increment_pc();
}

void Machine::do_fmul()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_div()
{
    NativeInt const dividend = rAX.native_value();
    Sign const dividend_sign = rAX.sign();
    
    NativeInt const divisor = inst.native_V();
    if (divisor == 0)
        throw std::runtime_error("Divide by zero");
    
    // TODO: Check for overflow

    // sgn(rAX / V) * floor(|rAX / V|)
    // Regular division already rounds toward zero, thereby achieving the desired effect.
    NativeInt const quotient = dividend / divisor;
    rA.load<false>(quotient);

    // sgn(rAX) * (|rAX| mod |V|)
    NativeInt const remainder = dividend_sign * (std::labs(dividend) % std::labs(divisor));
    rX.load<false>(remainder);

    increment_pc();
}

void Machine::do_fdiv()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_num()
{
    NativeInt value = 0;
    for (size_t i = 1; i < rA.reg.size(); i++)
    {
        NativeByte const character_value = rA.reg[i].byte % 10;
        value = value * byte_size + character_value;
    }

    for (size_t i = 1; i < rX.reg.size(); i++)
    {
        NativeByte const character_value = rX.reg[i].byte % 10;
        value = value * byte_size + character_value;
    }

    if (rA.sign() == s_minus)
        value = -value;

    if (rA.load<false>(value))
        std::cerr << "Warning: overflow during NUM\n";
    increment_pc();
}

void Machine::do_char()
{
    static constexpr auto thirty = to_interval(from_literal<30>());
    static constexpr auto ten = deduce_sequence<TypeSequence<IsInClosedInterval<0, byte_size - 1>, IsNonNegative>>(from_literal<10>());
    ValidatedNonNegative unsigned_value = rA.native_unsigned_value();
    for (size_t i = rX.reg.size(); i --> 1; unsigned_value = unsigned_value / ten)
        rX.reg[i].byte = ValidatedInt<IsInClosedInterval<0, byte_size - 1>>(thirty + ValidatedUtils::from_mod<10>(unsigned_value));

    for (size_t i = rA.reg.size(); i --> 1; unsigned_value = unsigned_value / ten)
        rA.reg[i].byte = ValidatedInt<IsInClosedInterval<0, byte_size - 1>>(thirty + ValidatedUtils::from_mod<10>(unsigned_value));

    if (unsigned_value > 0)
        std::cerr << "Warning: overflow in CHAR";
    increment_pc();
}

void Machine::do_hlt()
{
    halted = true;
}

void Machine::do_sla()
{
    rA.shift_left(inst.native_M());
}

void Machine::do_sra()
{
    rA.shift_right(inst.native_M());
}

void Machine::do_slax()
{
    rAX.shift_left(inst.native_M());
}

void Machine::do_srax()
{
    rAX.shift_right(inst.native_M());
}

void Machine::do_slc()
{
    rAX.shift_left_circular(inst.native_M());
}

void Machine::do_src()
{
    rAX.shift_right_circular(inst.native_M());
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_ld()
{
    return inst.MF().transform_value([this](SliceView const slice){
        RegisterT &reg = this->*reg_member_ptr;
        reg.template load<true>(slice.native_value());
    });
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_ldn()
{
    return inst.MF().transform_value([this](SliceView const slice){
        RegisterT &reg = this->*reg_member_ptr;
        reg.template load<true>(-slice.native_value());
    });
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_st()
{
    return inst.MF().transform_value([this](SliceMutable const slice){
        RegisterT const &reg = this->*reg_member_ptr;
        reg.store(slice);
    });
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_j()
{
    
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_inc()
{
    return inst.native_M().transform_value([this](NativeInt reg_addend){
        RegisterT &reg = this->*reg_member_ptr;
        if (reg.increment(reg_addend))
            overflow = true;
    });    
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_dec()
{
    return inst.native_M().transform_value([this](NativeInt const reg_subtractend){
        NativeInt const reg_addend = -reg_subtractend;
        RegisterT &reg = this->*reg_member_ptr;
        if (reg.increment(reg_addend))
            overflow = true;
    });    
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_ent()
{
    return inst.native_M().transform_value([this](NativeInt const new_reg_value){
        RegisterT &reg = this->*reg_member_ptr;
        if (new_reg_value == 0)
            reg.load_zero(inst.sign());
        else
            reg.template load<true>(new_reg_value);
    });    
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_enn()
{
    return inst.native_M().transform_value([this](NativeInt const M){
        RegisterT &reg = this->*reg_member_ptr;
        if (M == 0)
            reg.load_zero(-inst.sign());
        else
            reg.template load<true>(-M);
    });
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
Result<void> Machine::do_cmp()
{
    return inst.MF().transform_value([this](SliceView const mem_slice){
        RegisterT const &reg = this->*reg_member_ptr;
        IntegralContainer<OwnershipKind::view, RegisterT::is_signed_v, RegisterT::size_v> container = reg;
        SliceView const reg_slice(container, mem_slice.spec);
        comparison = reg_slice.native_value() <=> mem_slice.native_value();
    });    
}

Result<void> Machine::jump_table()
{
    switch (inst.C())
    {
#define DISPATCH8(base) \
    case base * 8 + 0: \
        dispatch_by_op_code<base * 8 + 0>(); break; \
    case base * 8 + 1: \
        dispatch_by_op_code<base * 8 + 1>(); break; \
    case base * 8 + 2: \
        dispatch_by_op_code<base * 8 + 2>(); break; \
    case base * 8 + 3: \
        dispatch_by_op_code<base * 8 + 3>(); break; \
    case base * 8 + 4: \
        dispatch_by_op_code<base * 8 + 4>(); break; \
    case base * 8 + 5: \
        dispatch_by_op_code<base * 8 + 5>(); break; \
    case base * 8 + 6: \
        dispatch_by_op_code<base * 8 + 6>(); break; \
    case base * 8 + 7: \
        dispatch_by_op_code<base * 8 + 7>(); break; \


    DISPATCH8(0);
    DISPATCH8(1);
    DISPATCH8(2);
    DISPATCH8(3);
    DISPATCH8(4);
    DISPATCH8(5);
    DISPATCH8(6);
    DISPATCH8(7);
    default:
        // Bad op code
        return Result<void>::failure();
    }
#undef DISPATCH8

    return Result<void>::success();
}

template <NativeByte op_code>
void Machine::dispatch_by_op_code()
{
#define OP_LIST_DISPATCH_ITERATOR(OP_NAME, OP_CODE, FUNC) \
    if constexpr(op_code == OP_CODE) \
    { \
        FUNC(); \
        return; \
    }

#define OP_LIST_FIELD_DISPATCH_ITERATOR(OP_NAME, OP_CODE, OP_FIELD, FUNC) \
    if constexpr(op_code == OP_CODE) \
    { \
        if (inst.F() == OP_FIELD)\
        { \
            FUNC(); \
            return; \
        } \
    }

#define OP_LIST_REGISTER_DISPATCH_ITERATOR(OP_NAME, OP_CODE, FUNC, REGISTER) \
    if constexpr(op_code == OP_CODE) \
    { \
        FUNC<decltype(std::declval<Machine>().REGISTER), &Machine::REGISTER>(); \
        return; \
    }

#define OP_LIST_FIELD_REGISTER_DISPATCH_ITERATOR(OP_NAME, OP_CODE, OP_FIELD, FUNC, REGISTER) \
    if constexpr(op_code == OP_CODE) \
    { \
        if (inst.F() == OP_FIELD) \
        { \
            FUNC<decltype(std::declval<Machine>().REGISTER), &Machine::REGISTER>(); \
            return; \
        } \
    }

    OP_LIST(OP_LIST_DISPATCH_ITERATOR, OP_LIST_FIELD_DISPATCH_ITERATOR, OP_LIST_REGISTER_DISPATCH_ITERATOR, OP_LIST_FIELD_REGISTER_DISPATCH_ITERATOR)

#undef OP_LIST_DISPATCH_ITERATOR
}

void Machine::step()
{
    jump_table();
}

}
