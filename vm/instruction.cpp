#include "base/math.impl.h"
#include "base/validation/v2.defn.h"
#include "base/validation/validator.impl.h"
#include "config.impl.h"
#include <base/base.h>
#include <base/validation/constants.h>
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

ValidatedWord Instruction::native_A() const
{
    auto const [
        s, 
        b1, 
        b2
    ] = A();

    ValidatedWord const word = ValidatedInt<IsInClosedInterval<mix_int_min, mix_int_max>>(
        ValidatedConstructors::multiply(
            ValidatedConstructors::add(
                ValidatedConstructors::multiply(
                    to_interval(b1),
                    to_interval(validated_byte_size)
                ),
                to_interval(b2)
            ),
            ValidatedConstructors::from_sign(s)
        )
    );
    return word;
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

ValidatedAddress Instruction::native_M() const
{
    NativeInt const base_address = native_A();
    NativeInt const offset = native_I_value_or_zero();
    NativeInt const address = base_address + offset;
    check_address_bounds(address);
    return address;
}

Word<OwnershipKind::mutable_view> Instruction::M_value() const
{
    ValidatedAddress address = native_M();
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

ValidatedWord Instruction::native_MF() const
{
    return MF().native_value();
}

}
