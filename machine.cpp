#include "base.h"
#include "machine.decl.h"
#include "register.decl.h"
#include <iostream>
#include <machine.h>
#include <stdexcept>

void Machine::do_nop()
{
    increment_pc();
}

void Machine::do_add()
{
    Instruction const inst = current_instruction();
    if (rA.store(rA.native_value() + inst.native_MF()))
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
    Instruction const inst = current_instruction();
    if (rA.store(rA.native_value() - inst.native_MF()))
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
    Instruction const inst = current_instruction();
    NativeInt const mul_result = rA.native_value() * inst.native_MF();
    ExtendedRegister rAX = get_rAX();
    rAX.store(mul_result);
    increment_pc();
}

void Machine::do_fmul()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_div()
{
    Instruction const inst = current_instruction();
    ExtendedRegister rAX = get_rAX();

    NativeInt const dividend = rAX.native_value();
    Sign const dividend_sign = rAX.sign();
    
    NativeInt const divisor = inst.native_V();
    if (divisor == 0)
        throw std::runtime_error("Divide by zero");
    
    // TODO: Check for overflow

    // sgn(rAX / V) * floor(|rAX / V|)
    // Regular division already rounds toward zero, thereby achieving the desired effect.
    NativeInt const quotient = dividend / divisor;
    rA.store(quotient);

    // sgn(rAX) * (|rAX| mod |V|)
    NativeInt const remainder = dividend_sign * (std::labs(dividend) % std::labs(divisor));
    rX.store(remainder);

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
    for (size_t i = 1; i < rA.arr.size(); i++)
    {
        NativeByte const character_value = rA.arr[i].byte % 10;
        value = value * byte_size + character_value;
    }

    for (size_t i = 1; i < rX.arr.size(); i++)
    {
        NativeByte const character_value = rX.arr[i].byte % 10;
        value = value * byte_size + character_value;
    }

    if (rA.sign() == s_minus)
        value = -value;

    if (rA.store(value))
        std::cerr << "Warning: overflow during NUM\n";
    increment_pc();
}

void Machine::do_char()
{
    NativeInt unsigned_value = rA.native_unsigned_value();
    for (size_t i = rX.arr.size(); i --> 1; unsigned_value /= 10)
        rX.arr[i].byte = 30 + unsigned_value % 10;

    for (size_t i = rA.arr.size(); i --> 1; unsigned_value /= 10)
        rA.arr[i].byte = 30 + unsigned_value % 10;

    if (unsigned_value > 0)
        std::cerr << "Warning: overflow in CHAR";
    increment_pc();
}

void Machine::do_hlt()
{
    halted = true;
}

Op Machine::current_op()
{
    Instruction inst = current_instruction();
    switch (inst.C())
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    }
}

void Machine::step()
{
    Op const op = current_op();
    op(*this);
}
