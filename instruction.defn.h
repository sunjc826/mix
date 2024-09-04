#pragma once
#include <base.h>
#include <instruction.decl.h>
#include <machine.decl.h>

struct Instruction : public Word<OwnershipKind::owns>
{
    Machine &m;
    Instruction(Machine &m);

    void update_by_pc();

    Sign sign() const
    {
        return container[0].sign;
    }

    std::tuple<Sign, NativeByte, NativeByte> A() const
    {
        return { container[0].sign, container[1].byte, container[2].byte };
    }

    NativeByte I() const
    {
        return container[3].byte;
    }

    // Returns *M, where * represents dereferencing
    Word<OwnershipKind::mutable_view> M_value() const;

    NativeByte F() const
    {
        return container[4].byte;
    }

    // Returns M(F)
    SliceMutable MF() const;

    NativeByte C() const
    {
        return container[5].byte;
    }

    // Extracts and returns L and R from F()
    FieldSpec field_spec() const;

    NativeInt native_A() const;

    // Returns rIi, where i is the value of I()
    NativeInt native_I_value_or_zero() const;

    // Returns M = A + rIi
    NativeInt native_M() const;

    // Returns native value of M(F)
    NativeInt native_MF() const;
    // Same as native M(F)
    __attribute__((always_inline))
    NativeInt native_V() const { return native_MF(); }
};