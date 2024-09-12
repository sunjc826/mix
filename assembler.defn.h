#pragma once
#include <base.h>
#include <error.h>

#include <istream>
#include <string_view>
#include <unordered_map>
#include <variant>

struct BufferedIStream
{
    std::istream input;
};

struct Cursor
{
    size_t line_number;
    size_t column_number;
    std::string_view full_line_segment, partial_line_segment;
    
    void process_next_line(std::string_view new_line)
    {
        full_line_segment = partial_line_segment = new_line;
        line_number++;
        column_number = 0;
    }

    void process_same_line(std::string_view rest_of_line)
    {
        column_number += partial_line_segment.size();
        full_line_segment = partial_line_segment = rest_of_line;
    }

    __attribute__((always_inline))
    bool empty()
    {
        return partial_line_segment.empty();
    }

    size_t length()
    {
        return partial_line_segment.size();
    }

    __attribute__((always_inline))
    char front()
    {
        return partial_line_segment.front();
    }

    template <bool assert_non_empty, bool consume_if_match, bool consume_if_not_match>
    bool check_impl(char ch)
    {
        if constexpr(!assert_non_empty)
        {
            if (empty())
                return false;
        }

        if (front() != ch)
        {
            if constexpr (consume_if_not_match)
                advance();    
            return false;
        }
        else
        {
            if constexpr (consume_if_match)
                advance();
        }
        
        if constexpr(consume_if_match)
            advance();

        return true;
    }

    template <bool assert_non_empty, bool consume_if_match>
    bool check(char ch)
    {
        return check_impl<assert_non_empty, consume_if_match, false>(ch);
    }

    template <bool assert_non_empty, bool consume_if_not_match>
    bool check_no_match(char ch)
    {
        return check_impl<assert_non_empty, false, consume_if_not_match>(ch);
    }

    template <bool assert_non_empty, bool consume_if_match, int ...ctype_predicates(int ch)>
    bool check()
    {
        if constexpr(!assert_non_empty)
        {
            if (empty())
                return false;
        }

        if (!(ctype_predicates(front()) | ...))
            return false;
        
        if constexpr(consume_if_match)
            advance();

        return true;
    }

    __attribute__((always_inline))
    void advance()
    {
        partial_line_segment.remove_prefix(1);
        column_number++;
    }

    void advance_by(size_t n)
    {
        partial_line_segment.remove_prefix(n);
        column_number += n;
    }

    void advance_until(char ch)
    {
        while (!empty() && check_no_match<true, true>(ch))
            ;
    }

    __attribute__((always_inline))
    char const *save_str_begin()
    {
        return partial_line_segment.begin();
    }

    __attribute__((always_inline))
    std::string_view saved_str_end(char const *saved_begin)
    {
        return {saved_begin, partial_line_segment.begin()};
    }
};

struct SymbolString
{
    std::string_view symbol;
};

struct NumberString
{
    std::string_view number;
};

struct EmptyString {};

struct FutureReference
{
    SymbolString symbol;
};

struct LiteralConstant
{
    NativeInt value;
};

struct AddressIndexField
{
    std::variant<NativeInt, LiteralConstant, FutureReference> A;
    ValidatedRegisterIndex I;
    std::optional<ValidatedByte> F;
};

enum class BinaryOp
{
    add,
    subtract,
    multiply,
    divide,
    double_slash,
    colon,
};

template <typename ValueT>
using SymbolTable = std::unordered_map<std::string, ValueT, string_hash, std::equal_to<void>>;
using ResolvedSymbolTable = SymbolTable<ValidatedWord>;
using UnresolvedSymbolTable = SymbolTable<void *>;

static constexpr ValidatedLiteral<0> zero = ValidatedLiteral<0>::constructor(0);
static constexpr ValidatedLiteral<1> one = ValidatedLiteral<1>::constructor(1);
static constexpr ValidatedLiteral<2> two = ValidatedLiteral<2>::constructor(2);
static constexpr ValidatedLiteral<3> three = ValidatedLiteral<3>::constructor(3);
static constexpr ValidatedLiteral<4> four = ValidatedLiteral<4>::constructor(4);
static constexpr ValidatedLiteral<5> five = ValidatedLiteral<5>::constructor(5);
static constexpr ValidatedLiteral<6> six = ValidatedLiteral<6>::constructor(6);

class ExpressionParser
{
    Cursor &cursor;
    ResolvedSymbolTable const &symbol_table;
    UnresolvedSymbolTable const &unresolved_symbols;    

    // Even if the evaluation overflows, the result of all binary operations are still well-defined.
    // For example, C <- A + B is defined as
    // `LDA AA; ADD BB; STA CC` where AA, BB, CC are addresses respectively containing the value of A, B, C.
    // `ADD` is still valid when overflowing.
    static 
    Result<ValidatedWord, Error>
    evaluate(ValidatedWord lhs, BinaryOp op, ValidatedWord rhs);

    std::optional<BinaryOp>
    try_parse_binary_op();

    // A symbol is defined as
    // a sequence of alphanumeric characters 
    std::variant<SymbolString, NumberString, EmptyString>
    try_parse_symbol_or_number();

    // An <atomic expression> is defined as one of
    // (1) <number>
    // (2) <defined symbol>
    // (3) <asterisk>
    // where a <number> is an unsigned integer literal. (Any sign is defined as part of <expression> instead.)
    // See <expression> for the value semantics of an atomic expression.
    // `try_...` refers to the possibility of having no atomic expression, 
    // in that case, std::nullopt is the result.
    Result<std::optional<ValidatedWord>, Error>
    try_parse_atomic_expression();

    // An <expression> is defined as one of
    // (1) <atomic expression>
    // (2) <atomic expression> <binary op> <expression>
    // where <binary op> is one of
    // +, -, *, /, //, :
    // There are no associativity rules and the expression is evaluated from left to right.
    // The evaluation of a binary op is defined using MIX operations. Thus, this implies that
    // we need to use MIX semantics to evaluate them.
    // Because all 6 binary operations (+, -, *, /, //, :) begin with `LDA AA` and end with `STA CC`,
    // to be consistent, we will also think of the value of an atomic expression as `LDA AA; STA AA`.
    // This implies that the value of an atomic expression must fall within the bounds of a MIX integer.
    Result<std::optional<ValidatedWord>, Error>
    try_parse_expression();

    // A <literal constant> is defined as
    // = <W value; length \lt 10> =
    // where <W value; length \lt 10> is defined as
    // a <W value> whose length is less than 10.
    Result<std::optional<LiteralConstant>, Error>
    try_parse_literal_constant();

    std::optional<FutureReference>
    try_parse_future_reference();

    Result<std::variant<NativeInt, LiteralConstant, FutureReference>, Error>
    parse_A_part();

    Result<ValidatedRegisterIndex, Error>
    parse_I_part();

    Result<std::optional<ValidatedByte>, Error>
    parse_F_part();

public:
    ExpressionParser(Cursor &cursor, ResolvedSymbolTable const &symbol_table, UnresolvedSymbolTable const &unresolved_symbols)
        : cursor(cursor), symbol_table(symbol_table), unresolved_symbols(unresolved_symbols)
    {}

    Result<AddressIndexField, Error>
    parse_AIF();

    Result<ValidatedWord, Error>
    parse_W_value();
};

class Assembler
{
    std::istream &assembly;
    std::ostream &binary;
    std::string line;
    Cursor cursor;
    ValidatedWord location_counter;
    ResolvedSymbolTable symbol_table;
    UnresolvedSymbolTable unresolved_symbols;

    Result<void, Error>
    advance_location_counter(ValidatedPositiveWord by = one);

    void 
    add_symbol(std::string_view loc);

    void 
    add_symbol(std::string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_equ(std::string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_orig(std::string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_con(std::string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_alf(std::string_view loc, std::string_view str);
    
    Result<void, Error>
    assemble_end(std::string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_instruction(std::string_view loc, AddressIndexField AIF);
    // Value:
    // - true if end of input
    // - false otherwise
    Result<bool, Error>
    assemble_line();
public:
    Assembler(std::istream &assembly, std::ostream &binary)
        : assembly(assembly), binary(binary), location_counter(zero)
    {}
    
    void
    assemble();
};
