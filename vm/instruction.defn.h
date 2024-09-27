#pragma once
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

    ValidatedByte I() const
    {
        return container[3].byte;
    }

    // Returns *M, where * represents dereferencing
    Word<OwnershipKind::mutable_view> M_value() const;

    ValidatedByte F() const
    {
        return container[4].byte;
    }

    // Returns M(F)
    SliceMutable MF() const;

    ValidatedByte C() const
    {
        return container[5].byte;
    }

    // Extracts and returns L and R from F()
    FieldSpec field_spec() const;

    ValidatedWord native_A() const;

    // Returns rIi, where i is the value of I()
    NativeInt native_I_value_or_zero() const;

    // Returns M = A + rIi
    ValidatedAddress native_M() const;

    // Returns native value of M(F)
    ValidatedWord native_MF() const;
    // Same as native M(F)
    [[gnu::flatten]]
    ValidatedWord native_V() const { return native_MF(); }
};

}
