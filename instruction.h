#pragma once
#include <base.h>
#include <instruction.decl.h>
#include <machine.decl.h>
struct Instruction : public Word
{
    Machine &m;
    std::tuple<Sign &, NativeByte &, NativeByte &> A() const
    {
        return { sp[0].sign, sp[1].byte, sp[2].byte };
    }

    NativeByte &I() const
    {
        return sp[3].byte;
    }

    // Returns *M, where * represents dereferencing
    std::span<Byte, bytes_in_word> M_value() const;

    NativeByte &F() const
    {
        return sp[4].byte;
    }

    // Returns M(F)
    Slice MF() const;

    NativeByte &C() const
    {
        return sp[5].byte;
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