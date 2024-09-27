#include "base/math.impl.h"
#include "base/validation/v2.decl.h"
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

ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>> Instruction::native_A() const
{
    auto const [
        s, 
        b1, 
        b2
    ] = A();

    return ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>(
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
}

Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>> Instruction::native_I_value_or_zero() const
{
    using ResultType = Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>>;
    NativeByte const b3 = I();
    if (b3 == 0)
        return ResultType::success(zero);
    if (b3 >= 6)
        return ResultType::failure();
    IndexRegister const &r = *m.get_index_register(b3);
    return ResultType::success(r.native_value());
}

Result<ValidatedAddress> Instruction::native_M() const
{
    ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>> const base_address = native_A();
    Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>> const offset = native_I_value_or_zero();
    if (!offset)
        return Result<ValidatedAddress>::failure();
    return ValidatedAddress::constructor(ValidatedConstructors::add(base_address, offset.value()));
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
