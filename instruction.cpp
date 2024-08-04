#include <instruction.h>
#include <register.h>

void Word::store(std::span<Byte, bytes_in_word> new_word)
{
    std::copy(new_word.begin(), new_word.end(), sp.begin());
}

NativeInt Instruction::native_A() const
{
    auto const [s, b1, b2] = A();
    NativeInt accumulator = 0;
    accumulator = accumulator * MIX_BYTE_SIZE + b1;
    accumulator = accumulator * MIX_BYTE_SIZE + b2;
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

std::span<Byte, bytes_in_word> Instruction::M_value() const
{
    NativeInt address = native_M();
    return m.get_memory_word(address);
}

// (L:R) is 8L + R
FieldSpec Instruction::field_spec() const
{
    NativeByte const &field = F();
    return { .L = field / 8, .R = field % 8 };
}

Slice Instruction::MF() const
{
    std::span<Byte, bytes_in_word> const value_at_address_M = M_value();
    FieldSpec const spec = field_spec();
    std::span<Byte> const subspan = value_at_address_M.subspan(spec.L, spec.length());
    return { .sp = subspan, .spec = spec };
}

NativeInt Instruction::native_MF() const
{
    return MF().native_value();
}
