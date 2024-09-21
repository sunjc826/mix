#pragma once
#include <base/base.h>
#include <vm/machine.decl.h>
#include <vm/register.defn.h>
#include <vm/instruction.defn.h>
namespace mix
{

#define OP_LIST(IT, IT_FIELD, IT_REGISTER, IT_FIELD_REGISTER) \
    IT(nop, 0, do_nop) \
    IT(add, 1, do_add) IT(fadd, 1, do_add) \
    IT(sub, 2, do_sub) IT(fsub, 2, do_sub) \
    IT(mul, 3, do_mul) IT(fmul, 3, do_mul) \
    IT(div, 4, do_div) IT(fdiv, 4, do_div) \
    IT_FIELD(num, 5, 0, do_num) IT_FIELD(char, 5, 1, do_char) IT_FIELD(hlt, 5, 2, do_hlt) \
    IT_FIELD(sla, 6, 0, do_sla) IT_FIELD(sra, 6, 1, do_sra) IT_FIELD(slax, 6, 2, do_slax) IT_FIELD(srax, 6, 3, do_srax) IT_FIELD(slc, 6, 4, do_slc) IT_FIELD(src, 6, 5, do_src) \
    IT(move, 7, do_move) \
    IT_REGISTER(lda, 8, do_ld, rA) \
    IT_REGISTER(ld1, 9, do_ld, rI1) \
    IT_REGISTER(ld2, 10, do_ld, rI2) \
    IT_REGISTER(ld3, 11, do_ld, rI3) \
    IT_REGISTER(ld4, 12, do_ld, rI4) \
    IT_REGISTER(ld5, 13, do_ld, rI5) \
    IT_REGISTER(ld6, 14, do_ld, rI6) \
    IT_REGISTER(ldx, 15, do_ld, rX) \
    IT_REGISTER(ldan, 16, do_ldn, rA) \
    IT_REGISTER(ld1n, 17, do_ldn, rI1) \
    IT_REGISTER(ld2n, 18, do_ldn, rI2) \
    IT_REGISTER(ld3n, 19, do_ldn, rI3) \
    IT_REGISTER(ld4n, 20, do_ldn, rI4) \
    IT_REGISTER(ld5n, 21, do_ldn, rI5) \
    IT_REGISTER(ld6n, 22, do_ldn, rI6) \
    IT_REGISTER(ldxn, 23, do_ldn, rX) \
    IT_REGISTER(sta, 24, do_st, rA) \
    IT_REGISTER(st1, 25, do_st, rI1) \
    IT_REGISTER(st2, 26, do_st, rI2) \
    IT_REGISTER(st3, 27, do_st, rI3) \
    IT_REGISTER(st4, 28, do_st, rI4) \
    IT_REGISTER(st5, 29, do_st, rI5) \
    IT_REGISTER(st6, 30, do_st, rI6) \
    IT_REGISTER(stx, 31, do_st, rX) \
    IT_REGISTER(stj, 32, do_st, rJ) \
    IT_REGISTER(stz, 33, do_st, rZ) \
    IT(jbus, 34, do_jbus) \
    IT(ioc, 35, do_ioc) \
    IT(in, 36, do_in) \
    IT(out, 37, do_out) \
    IT(jred, 38, do_jred) \
    IT_FIELD(jmp, 39, 0, do_jmp) IT_FIELD(jsj, 39, 1, do_jsj) IT_FIELD(jov, 39, 2, do_jov) IT_FIELD(jnov, 39, 3, do_jnov) \
    IT_REGISTER(ja, 40, do_j, rA) \
    IT_REGISTER(j1, 41, do_j, rI1) \
    IT_REGISTER(j2, 42, do_j, rI2) \
    IT_REGISTER(j3, 43, do_j, rI3) \
    IT_REGISTER(j4, 44, do_j, rI4) \
    IT_REGISTER(j5, 45, do_j, rI5) \
    IT_REGISTER(j6, 46, do_j, rI6) \
    IT_REGISTER(jx, 47, do_j, rX) \
    IT_FIELD_REGISTER(inca, 48, 0, do_inc, rA) IT_FIELD_REGISTER(deca, 48, 1, do_dec, rA) IT_FIELD_REGISTER(enta, 48, 2, do_ent, rA) IT_FIELD_REGISTER(enna, 48, 3, do_enn, rA) \
    IT_FIELD_REGISTER(inc1, 49, 0, do_inc, rI1) IT_FIELD_REGISTER(dec1, 49, 1, do_dec, rI1) IT_FIELD_REGISTER(ent1, 49, 2, do_ent, rI1) IT_FIELD_REGISTER(enn1, 49, 3, do_enn, rI1) \
    IT_FIELD_REGISTER(inc2, 50, 0, do_inc, rI2) IT_FIELD_REGISTER(dec2, 50, 1, do_dec, rI2) IT_FIELD_REGISTER(ent2, 50, 2, do_ent, rI2) IT_FIELD_REGISTER(enn2, 50, 3, do_enn, rI2) \
    IT_FIELD_REGISTER(inc3, 51, 0, do_inc, rI3) IT_FIELD_REGISTER(dec3, 51, 1, do_dec, rI3) IT_FIELD_REGISTER(ent3, 51, 2, do_ent, rI3) IT_FIELD_REGISTER(enn3, 51, 3, do_enn, rI3) \
    IT_FIELD_REGISTER(inc4, 52, 0, do_inc, rI4) IT_FIELD_REGISTER(dec4, 52, 1, do_dec, rI4) IT_FIELD_REGISTER(ent4, 52, 2, do_ent, rI4) IT_FIELD_REGISTER(enn4, 52, 3, do_enn, rI4) \
    IT_FIELD_REGISTER(inc5, 53, 0, do_inc, rI5) IT_FIELD_REGISTER(dec5, 53, 1, do_dec, rI5) IT_FIELD_REGISTER(ent5, 53, 2, do_ent, rI5) IT_FIELD_REGISTER(enn5, 53, 3, do_enn, rI5) \
    IT_FIELD_REGISTER(inc6, 54, 0, do_inc, rI6) IT_FIELD_REGISTER(dec6, 54, 1, do_dec, rI6) IT_FIELD_REGISTER(ent6, 54, 2, do_ent, rI6) IT_FIELD_REGISTER(enn6, 54, 3, do_enn, rI6) \
    IT_FIELD_REGISTER(incx, 55, 0, do_inc, rX) IT_FIELD_REGISTER(decx, 55, 1, do_dec, rX) IT_FIELD_REGISTER(entx, 55, 2, do_ent, rX) IT_FIELD_REGISTER(ennx, 55, 3, do_enn, rX) \
    IT_REGISTER(cmpa, 56, do_cmp, rA) IT_REGISTER(fcmp, 56, do_cmp, rA) \
    IT_REGISTER(cmp1, 57, do_cmp, rI1) \
    IT_REGISTER(cmp2, 58, do_cmp, rI2) \
    IT_REGISTER(cmp3, 59, do_cmp, rI3) \
    IT_REGISTER(cmp4, 60, do_cmp, rI4) \
    IT_REGISTER(cmp5, 61, do_cmp, rI5) \
    IT_REGISTER(cmp6, 62, do_cmp, rI6) \
    IT_REGISTER(cmpx, 63, do_cmp, rX) 

enum OpCode : NativeByte 
{
#define OP_LIST_ENUM_ITERATOR(OP_NAME, OP_CODE, ...) op_##OP_NAME = OP_CODE,
    OP_LIST(OP_LIST_ENUM_ITERATOR, OP_LIST_ENUM_ITERATOR, OP_LIST_ENUM_ITERATOR, OP_LIST_ENUM_ITERATOR)
#undef OP_LIST_ENUM_ITERATOR
    op_max,
};

static_assert(op_max == minimum_byte_size);

// Represents the MIX machine state
class Machine
{
    friend class Instruction;
    template <bool, size_t> 
    friend struct Register;

    // program counter
    NativeByte pc;
    
#define REGISTER_LIST(IT, ...) /* ... are additional args */ \
    IT(NumberRegister, rA, __VA_ARGS__) \
    IT(IndexRegister, rI1, __VA_ARGS__) \
    IT(IndexRegister, rI2, __VA_ARGS__) \
    IT(IndexRegister, rI3, __VA_ARGS__) \
    IT(IndexRegister, rI4, __VA_ARGS__) \
    IT(IndexRegister, rI5, __VA_ARGS__) \
    IT(IndexRegister, rI6, __VA_ARGS__) \
    IT(NumberRegister, rX, __VA_ARGS__) \
    IT(JumpRegister, rJ, __VA_ARGS__) \
    IT(ZeroRegister, rZ, __VA_ARGS__)

#define REGISTER_DECLARATION_ITERATOR(TYPE, REG, ...) TYPE REG;
    REGISTER_LIST(REGISTER_DECLARATION_ITERATOR)
#undef REGISTER_DECLARATION_ITERATOR
    ExtendedRegister rAX{rA, rX};

    enum RegisterIdx
    {
#define REGISTER_ENUM_ITERATOR(TYPE, REG, ...) idx_##REG,
        REGISTER_LIST(REGISTER_ENUM_ITERATOR)
#undef REGISTER_ENUM_ITERATOR
    };

    bool halted;

    // overflow toggle
    bool overflow;

    // comparison indicator
    std::strong_ordering comparison{std::strong_ordering::equal};  
    
    // A MIX machine has 4000 memory cells
    std::array<Byte, main_memory_size * bytes_in_word> memory;

    Instruction inst{*this}; // current instruction

    __attribute__((always_inline)) inline
    void update_current_instruction();

    __attribute__((always_inline)) inline
    void increment_pc();

    __attribute__((always_inline)) inline
    std::optional<std::reference_wrapper<IndexRegister>> get_index_register(NativeByte index);

    __attribute__((always_inline)) inline
    Word<OwnershipKind::mutable_view> get_memory_word(NativeInt address);

    template <NativeByte test_op_code = 0>
    __attribute__((flatten))
    void jump_table();

    template <NativeByte op_code>
    __attribute__((always_inline, flatten))
    void dispatch_by_op_code();

    void do_nop();
    void do_add();
    void do_fadd();
    void do_sub();
    void do_fsub();
    void do_mul();
    void do_fmul();
    void do_div();
    void do_fdiv();
    void do_num();
    void do_char();
    void do_hlt();
    void do_sla();
    void do_sra();
    void do_slax();
    void do_srax();
    void do_slc();
    void do_src();
    void do_move();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_ld();
    
    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_ldn();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_st();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_j();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_inc();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_dec();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_ent();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_enn();

    template <typename RegisterT, RegisterT Machine::*reg_member_ptr>
    void do_cmp();

    void do_in();
    void do_out();
    void do_jbus();
    void do_jred();
    void do_jmp();
    void do_jov();
    void do_jnov();
    void do_jsj();
    void do_ioc();

public:
    Machine() = default;
    void step();
};

}
