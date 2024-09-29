#pragma once
#include <base/base.h>
#include <vm/machine.defn.h>
namespace mix
{

void Machine::update_current_instruction()
{
    inst.update_by_pc();
}

void Machine::increment_pc()
{
    pc += bytes_in_word;
}

IndexRegister &Machine::get_index_register(ValidatedRegisterIndex index)
{
    return index_registers[index - 1];
}

Word<OwnershipKind::mutable_view> Machine::get_memory_word(ValidatedAddress address)
{
    return {std::span<Byte, bytes_in_word>{memory.begin() + address * bytes_in_word, bytes_in_word}};
}

}
