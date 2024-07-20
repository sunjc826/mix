#include <cmath>
#include <config.h>
#include <utilities.h>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <span>
#include <type_traits>
#include <tuple>

// A native byte always represents 256 values
constexpr size_t native_byte_size = 256;

// A MIX byte must represent at least 64 values
constexpr size_t minimum_byte_size = 64;

// A MIX word comprises 1 sign and 5 numerical bytes
constexpr size_t bytes_in_word = 6;
constexpr size_t numerical_bytes_in_word = bytes_in_word - 1;

// The actual configured size of a MIX byte
constexpr size_t byte_size = MIX_BYTE_SIZE;

// The configured size of a MIX byte must be greater or equal to minimum_byte_size
static_assert(byte_size >= minimum_byte_size);

template <typename T>
constexpr size_t representable_values_v = sizeof(T) * native_byte_size;

using ByteInt = unsigned;

// Instead of doing big integer arithmetic, let us
// require that NativeInt is large enough to hold the largest value of 
// any representable integral value.
using NativeInt = long long;

// The native size of ByteInt must be at least at large as the configured size of a MIX byte 
static_assert(representable_values_v<ByteInt> >= byte_size);

// Instead of providing a generalized constexpr integral pow function,
// let's only compute powers up to 11 due to rAX.
// Higher powers are unnecessary.
static __attribute__((always_inline))
constexpr std::array<NativeInt, 2 * numerical_bytes_in_word + 1> 
pow_lookup_table(ByteInt base)
{
    std::array<NativeInt, 2 * numerical_bytes_in_word + 1> lut;
    lut[0] = 1;
    for (size_t i = 1; i < lut.size(); i++)
        lut[i] = lut[i - 1] * base;
    return lut;
}

constexpr auto lut = pow_lookup_table(byte_size);

static __attribute__((always_inline))
constexpr NativeInt
pow(ByteInt base, size_t exponent)
{
    return pow_lookup_table(base)[exponent];
}

template <size_t exponent>
static __attribute__((always_inline))
constexpr NativeInt
pow(ByteInt base)
{
    static_assert(0 <= exponent && exponent <= 2 * numerical_bytes_in_word);
    return pow(base, exponent);
}

// Every negative MIX integral value must be representable by NativeInt
static_assert(-(lut[numerical_bytes_in_word] - 1) >= std::numeric_limits<NativeInt>::min());
// Every positive MIX integral value must be representable by NativeInt
static_assert(lut[numerical_bytes_in_word] - 1 <= std::numeric_limits<NativeInt>::max());

// prefer enum over enum class
enum Sign : ByteInt
{
    s_plus = 0,
    s_minus = 1,
};

static __attribute__((always_inline))
constexpr NativeInt 
native_sign(Sign sign)
{
    return sign == s_plus ? 1 : -1; 
}

union Byte
{
    ByteInt byte;
    Sign sign;
};

struct Word
{
    std::span<Byte, bytes_in_word> sp;
};

struct Instruction : public Word
{
    std::tuple<Sign &, ByteInt &, ByteInt &> A()
    {
        return { sp[0].sign, sp[1].byte, sp[2].byte };
    }

    ByteInt &I()
    {
        return sp[3].byte;
    }

    ByteInt &F()
    {
        return sp[4].byte;
    }

    ByteInt &C()
    {
        return sp[5].byte;
    }
};


template <bool is_signed, size_t size>
struct Int
{
    static constexpr size_t num_begin = is_signed ? 1 : 0;
    // If there is a sign byte, then the total number of bytes must > 1,
    // since there must be at least 1 numerical byte.
    static_assert(!is_signed || size > 1);
    std::span<Byte, size> sp;
    NativeInt native_sign() const
    {
        return ::native_sign(sign());
    }

    std::conditional_t<is_signed, Sign &, Sign> sign()
    {
        if constexpr(is_signed)
            return sp[0].sign;
        else
            return s_plus;
    }

    std::conditional_t<is_signed, Sign const &, Sign> sign() const
    {
        return REMOVE_CONST_FROM_PTR(this)->sign();
    }

    NativeInt native_value() const
    {
        NativeInt accum = 0;
        for (size_t i = is_signed; i < size; i++)
            accum += lut[i] * sp[i].value;
        return native_sign() * accum;
    }
};


struct NumberRegister
{
    std::array<Byte, 6> arr;
    Sign &sign()
    {
        return arr[0].sign;
    }

    Int<true, 6> value()
    {
        return {.sp = arr};
    }
};

struct IndexRegister
{
    std::array<Byte, 3> arr;
    Sign &sign()
    {
        return arr[0].sign;
    }
    
    Int<true, 3> value()
    {
        return {.sp = arr};
    }
};

struct JumpRegister
{
    std::array<Byte, 2> arr;

    Int<false, 2> value()
    {
        return {.sp = arr};
    }
};

enum CompareResult
{
    cr_less,
    cr_equal,
    cr_greater,
};

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

enum OpCode : ByteInt 
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

class Machine;
struct Op
{
    std::string_view const name;
    void (Machine::*const do_op)();
    void operator()(Machine &m) const
    {
        (m.*do_op)();
    }
};


// Represents the MIX machine state
class Machine
{
    // program counter
    ByteInt pc;

    NumberRegister rA, rX;
    IndexRegister rI1, rI2, rI3, rI4, rI5, rI6;
    JumpRegister rJ;

    // overflow toggle
    bool overflow;

    // comparison indicator
    CompareResult comparison;  
    
    // A MIX machine has 4000 memory cells
    std::array<Byte, 4000> memory;

    std::tuple<NumberRegister &, NumberRegister &> rAX()
    {
        return {rA, rX};
    }

    Instruction current_instruction()
    {
        return Instruction{Word{.sp = std::span<Byte, 6>( memory.begin() + pc, 6 )}};
    }

    void increment_pc()
    {
        pc += bytes_in_word;
    }

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
    void do_hlt();
    void do_sla();
    void do_sra();
    void do_slax();
    void do_srax();
    void do_slc();
    
public:
    Op current_op();
    void step();

};
