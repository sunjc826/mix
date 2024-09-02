#pragma once
#include <machine.defn.h>

__attribute__((always_inline)) inline
void Machine::update_current_instruction()
{
    inst.update_by_pc();
}

__attribute__((always_inline)) inline
void Machine::increment_pc()
{
    pc += bytes_in_word;
}

__attribute__((always_inline)) inline
std::optional<std::reference_wrapper<IndexRegister>> Machine::get_index_register(NativeByte index)
{
    switch (index)
    {
    case 1:
        return rI1;
    case 2:
        return rI2;
    case 3:
        return rI3;
    case 4:
        return rI4;
    case 5:
        return rI5;
    case 6:
        return rI6;
    default:
        return {};
    }
}

__attribute__((always_inline)) inline
std::span<Byte, 6> Machine::get_memory_word(NativeInt address)
{
    check_address_bounds(address);
    return std::span<Byte, 6>{memory.begin() + address * bytes_in_word, bytes_in_word};
}
