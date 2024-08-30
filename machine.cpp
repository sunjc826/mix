#include <base.h>
#include <instruction.h>
#include <machine.decl.h>
#include <register.decl.h>
#include <compare>
#include <iostream>
#include <machine.h>
#include <stdexcept>

void Machine::do_nop()
{
    increment_pc();
}

void Machine::do_add()
{
    if (rA.load(rA.native_value() + inst.native_MF()))
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
    if (rA.load(rA.native_value() - inst.native_MF()))
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
    rA.load(quotient);

    // sgn(rAX) * (|rAX| mod |V|)
    NativeInt const remainder = dividend_sign * (std::labs(dividend) % std::labs(divisor));
    rX.load(remainder);

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

    if (rA.load(value))
        std::cerr << "Warning: overflow during NUM\n";
    increment_pc();
}

void Machine::do_char()
{
    NativeInt unsigned_value = rA.native_unsigned_value();
    for (size_t i = rX.reg.size(); i --> 1; unsigned_value /= 10)
        rX.reg[i].byte = 30 + unsigned_value % 10;

    for (size_t i = rA.reg.size(); i --> 1; unsigned_value /= 10)
        rA.reg[i].byte = 30 + unsigned_value % 10;

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
void Machine::do_ld()
{
    SliceView const slice = inst.MF();
    RegisterT &reg = this->*reg_member_ptr;
    reg.load_throw_on_overflow(slice.native_value());
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_ldn()
{
    SliceView const slice = inst.MF();
    RegisterT &reg = this->*reg_member_ptr;
    reg.load_throw_on_overflow(-slice.native_value());
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_st()
{
    SliceMutable const slice = inst.MF();
    RegisterT &reg = this->*reg_member_ptr;
    reg.store(slice);
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_j()
{
    
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_inc()
{
    NativeInt const reg_addend = inst.native_M();
    RegisterT &reg = this->*reg_member_ptr;
    if (reg.increment(reg_addend))
        overflow = true;
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_dec()
{
    NativeInt const reg_addend = -inst.native_M();
    RegisterT &reg = this->*reg_member_ptr;
    if (reg.increment(reg_addend))
        overflow = true;
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_ent()
{
    NativeInt const new_reg_value = inst.native_M();
    RegisterT &reg = this->*reg_member_ptr;
    if (new_reg_value == 0)
        reg.load_zero(inst.sign());
    else
        reg.load_throw_on_overflow(new_reg_value);
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_enn()
{
    NativeInt const M = inst.native_M();
    RegisterT &reg = this->*reg_member_ptr;
    if (M == 0)
        reg.load_zero(-inst.sign());
    else
        reg.load_throw_on_overflow(-M);
}

template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
void Machine::do_cmp()
{
    SliceView const mem_slice = inst.MF();
    RegisterT &reg = this->*reg_member_ptr;
    SliceView const reg_slice = reg.make_slice(mem_slice.spec);
    comparison = reg_slice.native_value() <=> mem_slice.native_value();
}

template <NativeByte test_op_code>
void Machine::jump_table()
{
    if (inst.C() == test_op_code)
        dispatch_by_op_code<test_op_code>();
    else if constexpr (test_op_code + 1 < op_max)
        jump_table<test_op_code + 1>();
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
