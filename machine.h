#pragma once
#include <machine.decl.h>
#include <register.h>
#include <instruction.h>

constexpr std::array character_set
{
    " ",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "Δ",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "Σ",
    "Π",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ".",
    ",",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "=",
    "$",
    "<",
    ">",
    "@",
    ";",
    ":",
    "'",
};

enum OpCode : NativeByte 
{
    op_nop = 0,
    op_add = 1,
    op_fadd = 1,
    op_sub = 2,
    op_fsub = 2,
    op_mul = 3,
    op_fmul = 3,
    op_div = 4,
    op_fdiv = 4,
    op_num = 5,
    op_char = 5,
    op_hlt = 5,
    op_sla = 6,
    op_sra = 6,
    op_slax = 6,
    op_srax = 6,
    op_slc = 6,
    op_src = 6,
    op_move = 7,
    op_lda = 8,
    op_ld1,
    op_ld2,
    op_ld3,
    op_ld4,
    op_ld5,
    op_ld6,
    op_ldx,
    op_ldan = 16,
    op_ld1n,
    op_ld2n,
    op_ld3n,
    op_ld4n,
    op_ld5n,
    op_ld6n,
    op_ldxn,
    op_sta = 24,
    op_st1,
    op_st2,
    op_st3,
    op_st4,
    op_st5,
    op_st6,
    op_stx,
    op_stj = 32,
    op_stz = 33,
    op_jbus = 34,
    op_ioc = 35,
    op_in = 36,
    op_out = 37,
    op_jred = 38,
    op_jmp= 39,
    op_jsj = 39,
    op_jov = 39,
    op_jnov = 39,
    op_ja = 40,
    op_j1,
    op_j2,
    op_j3,
    op_j4,
    op_j5,
    op_j6,
    op_jx,
    op_inca = 48,
    op_inc1,
    op_inc2,
    op_inc3,
    op_inc4,
    op_inc5,
    op_inc6,
    op_incx,
    op_deca = 48,
    op_dec1,
    op_dec2,
    op_dec3,
    op_dec4,
    op_dec5,
    op_dec6,
    op_decx,
    op_enta = 48,
    op_ent1,
    op_ent2,
    op_ent3,
    op_ent4,
    op_ent5,
    op_ent6,
    op_entx,
    op_enna = 48,
    op_enn1,
    op_enn2,
    op_enn3,
    op_enn4,
    op_enn5,
    op_enn6,
    op_ennx,
    op_cmpa = 56,
    op_fcmp = 56,
    op_cmp1,
    op_cmp2,
    op_cmp3,
    op_cmp4,
    op_cmp5,
    op_cmp6,
    op_cmpx,
    op_max /* not an op */,
};
struct Op
{
    std::string_view const name;
    void (Machine::*const do_op)();
    __attribute__((always_inline)) inline 
    void operator()(Machine &m) const;
};
// Represents the MIX machine state
class Machine
{
    friend class Instruction;
    template <bool, size_t> 
    friend struct Register;

    // program counter
    NativeByte pc;

    NumberRegister rA, rX;
    IndexRegister rI1, rI2, rI3, rI4, rI5, rI6;
    JumpRegister rJ;
    ExtendedRegister rAX{rA, rX};
    std::array<TypeErasedRegister *, 9> register_list = 
        { &rA, &rI1, &rI2, &rI3, &rI4, &rI5, &rI6, &rX, &rJ };

    bool halted;

    // overflow toggle
    bool overflow;

    // comparison indicator
    std::strong_ordering comparison{std::strong_ordering::equal};  
    
    // A MIX machine has 4000 memory cells
    std::array<Byte, main_memory_size * bytes_in_word> memory;

    __attribute__((always_inline)) inline
    Instruction current_instruction();

    __attribute__((always_inline)) inline
    void increment_pc();

    __attribute__((always_inline)) inline
    std::optional<std::reference_wrapper<IndexRegister>> get_index_register(NativeByte index);

    __attribute__((always_inline)) inline
    std::span<Byte, 6> get_memory_word(NativeInt address);

    void do_nop();
    // rA <- rA + M(F)
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
    void do_ld(size_t register_idx);
    void do_ldn(size_t register_idx);
    void do_st(size_t register_idx);
    void do_stz();
    void do_j(size_t register_idx);
    void do_inc(size_t register_idx);
    void do_dec(size_t register_idx);
    void do_ent(size_t register_idx);
    void do_enn(size_t register_idx);
    void do_cmp(size_t register_idx);
public:
    Machine() = default;
    Op current_op();
    void step();

};

#include <machine.impl.h>
