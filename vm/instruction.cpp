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
        (
            (
                to_interval(b1) * to_interval(validated_byte_size)
            ) + to_interval(b2)
        ) * ValidatedUtils::from_sign(s)
    );
}

Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>> Instruction::native_I_value_or_zero() const
{
    return I().transform_value([this](ValidatedIValue idx) {
        return ValidatedUtils::visit(
            ValidatedIValue::raw_type(idx),
            [](ValidatedInt<IsExactValue<0>>){ 
                return zero; 
            },
            [this](ValidatedInt<IsRegisterIndex> idx) { 
                IndexRegister const &r = m.get_index_register(idx); 
                return r.native_value(); 
            }
        );
    });
}

Result<ValidatedAddress> Instruction::native_M() const
{
    ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>> const base_address = native_A();
    Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>> const offset = native_I_value_or_zero();
    if (!offset)
        return Result<ValidatedAddress>::failure();
    return ValidatedAddress::constructor(base_address + offset.value());
}

Result<Word<OwnershipKind::mutable_view>> Instruction::M_value() const
{
    using ResultType = Result<Word<OwnershipKind::mutable_view>>;
    Result<ValidatedAddress> const address = native_M();
    if (!address) { return ResultType::failure(); }
    return ResultType::success(m.get_memory_word(address.value()));
}

// (L:R) is 8L + R
FieldSpec Instruction::field_spec() const
{
    NativeByte const field = F();
    return FieldSpec::from_byte(field);
}

Result<SliceMutable> Instruction::MF() const
{
    using ResultType = Result<SliceMutable>;
    Result<Word<OwnershipKind::mutable_view>> const value_at_address_M = M_value();
    if (!value_at_address_M)
        return ResultType::failure();
    return ResultType::success(SliceMutable(value_at_address_M.value(), field_spec()));
}

Result<ValidatedWord> Instruction::native_MF() const
{
    using ResultType = Result<ValidatedWord>;
    Result<SliceMutable> slice = MF();
    if (!slice)
        return ResultType::failure();
    return ResultType::success(slice.value().native_value());
}

}
