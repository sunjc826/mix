#pragma once
#include "base/validation/v2.decl.h"
#include <base/base.h>
#include <base/validation/v2.h>
#include <vm/instruction.decl.h>
#include <vm/machine.decl.h>
namespace mix
{

struct Instruction : public Word<OwnershipKind::owns>
{
    Machine &m;
    Instruction(Machine &m);

    void update_by_pc();

    Sign sign() const
    {
        return container[0].sign;
    }

    std::tuple<Sign, ValidatedByte, ValidatedByte> A() const
    {
        return { container[0].sign, container[1].byte, container[2].byte };
    }

    Result<ValidatedIValue> I() const
    {
        return ValidatedIValue::constructor(container[3].byte);
    }

    // Returns *M, where * represents dereferencing
    Result<Word<OwnershipKind::mutable_view>> M_value() const;

    ValidatedByte F() const
    {
        return container[4].byte;
    }

    // Returns M(F)
    Result<SliceMutable> MF() const;

    ValidatedByte C() const
    {
        return container[5].byte;
    }

    // Extracts and returns L and R from F()
    FieldSpec field_spec() const;

    ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>> native_A() const;

    // Returns rIi, where i is the value of I()
    Result<ValidatedInt<IsInClosedInterval<-(lut[2] - 1), lut[2] - 1>>> native_I_value_or_zero() const;

    // Returns M = A + rIi
    Result<ValidatedAddress> native_M() const;

    // Returns native value of M(F)
    Result<ValidatedWord> native_MF() const;
    // Same as native M(F)
    [[gnu::flatten]]
    Result<ValidatedWord> native_V() const { return native_MF(); }
};

}
