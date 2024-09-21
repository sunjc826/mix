#include <base/base.h>
#include <vm/instruction.h>
#include <vm/register.h>
#include <vm/machine.h>

namespace mix
{

Instruction::Instruction(Machine &m)
    : Word(std::span<Byte, bytes_in_word>(m.memory.begin() + m.pc, bytes_in_word)), m(m)
{}

void Instruction::update_by_pc()
{
    std::copy_n(m.memory.begin() + m.pc, bytes_in_word, container.begin());
}

NativeInt Instruction::native_A() const
{
    auto const [s, b1, b2] = A();
    NativeInt accumulator = 0;
    accumulator = accumulator * byte_size + b1;
    accumulator = accumulator * byte_size + b2;
    accumulator = accumulator * s;
    return accumulator;
}

NativeInt Instruction::native_I_value_or_zero() const
{
    NativeByte const b3 = I();
    if (b3 == 0)
        return 0;
    if (b3 >= 6)
        throw std::runtime_error("invalid index");
    IndexRegister const &r = *m.get_index_register(b3);
    return r.native_value();
}

NativeInt Instruction::native_M() const
{
    NativeInt const base_address = native_A();
    NativeInt const offset = native_I_value_or_zero();
    NativeInt const address = base_address + offset;
    check_address_bounds(address);
    return address;
}

Word<OwnershipKind::mutable_view> Instruction::M_value() const
{
    NativeInt address = native_M();
    return m.get_memory_word(address);
}

// (L:R) is 8L + R
FieldSpec Instruction::field_spec() const
{
    NativeByte const field = F();
    return FieldSpec::from_byte(field);
}

SliceMutable Instruction::MF() const
{
    Word<OwnershipKind::mutable_view> const value_at_address_M = M_value();
    return SliceMutable(value_at_address_M, field_spec());
}

NativeInt Instruction::native_MF() const
{
    return MF().native_value();
}

}
