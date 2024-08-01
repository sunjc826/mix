#include <machine.h>

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

NativeInt Instruction::native_A()
{
    auto const [s, b1, b2] = A();
    NativeInt accumulator = 0;
    accumulator = accumulator * MIX_BYTE_SIZE + b1;
    accumulator = accumulator * MIX_BYTE_SIZE + b2;
    accumulator = accumulator * s;
    return accumulator;
}

NativeInt Instruction::native_I_value_or_zero()
{
    ByteInt b3 = I();
    if (b3 == 0)
        return 0;
    if (b3 >= 6)
        throw std::runtime_error("invalid index");
    IndexRegister const &r = *m.get_index_register(b3);
    return r.native_value();
}

NativeInt Instruction::native_M()
{
    NativeInt base_address = native_A();
    NativeInt offset = native_I_value_or_zero();
    NativeInt address = base_address + offset;
    check_address_bounds(address);
    return address;
}

std::span<Byte, bytes_in_word> Instruction::M_value()
{
    NativeInt address = native_M();
    return m.get_memory_word(address);
}

// (L:R) is 8L + R
FieldSpec Instruction::field_spec()
{
    ByteInt const &field = F();
    return { .L = field / 8, .R = field % 8 };
}

Slice Instruction::MF()
{
    std::span<Byte, bytes_in_word> const value_at_address_M = M_value();
    FieldSpec const spec = field_spec();
    std::span<Byte> const subspan = value_at_address_M.subspan(spec.L, spec.length());
    return { .sp = subspan, .spec = spec };
}

NativeInt Slice::native_value() const
{
    if (spec.L == 0)
    {
        Int<true> mix_int{.sp = sp};
        return mix_int.native_value();
    }
    else
    {
        Int<false> mix_int{.sp = sp};
        return mix_int.native_value();
    }
}

NativeInt Instruction::native_MF()
{
    return MF().native_value();
}

void Machine::do_nop()
{
    increment_pc();
}

// rA + M(F)
void Machine::do_add()
{
    Instruction inst = current_instruction();
    increment_pc();
}

void Machine::do_fadd()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_sub()
{
    increment_pc();
}

void Machine::do_fsub()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_mul()
{
    increment_pc();
}

void Machine::do_fmul()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_div()
{
    increment_pc();
}

void Machine::do_fdiv()
{
    throw std::runtime_error("Unimplemented");
    increment_pc();
}

void Machine::do_num()
{

}

void Machine::do_hlt()
{
    
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
