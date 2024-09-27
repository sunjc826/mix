#pragma once
#include "base/validation/validator.impl.h"
#include <base/base.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/io.h>
#include <base/string.h>

#include <istream>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace mix
{

struct Cursor
{
    size_t line_number;
    size_t column_number;
    string_view partial_line_segment;
    
    void process_next_line(string_view new_line)
    {
        partial_line_segment = new_line;
        line_number++;
        column_number = 0;
    }

    void process_same_line(string_view rest_of_line)
    {
        column_number += partial_line_segment.size();
        partial_line_segment = rest_of_line;
    }

    [[gnu::always_inline]]
    bool empty()
    {
        return partial_line_segment.empty();
    }

    [[gnu::always_inline]]
    size_t length()
    {
        return partial_line_segment.size();
    }

    [[gnu::always_inline]]
    NativeByte front()
    {
        return partial_line_segment.front();
    }

    template <bool assert_non_empty, bool consume_if_match, bool consume_if_not_match>
    bool check_impl(ValidatedChar ch)
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
    bool check(ValidatedChar ch)
    {
        return check_impl<assert_non_empty, consume_if_match, false>(ch);
    }

    template <bool assert_non_empty, bool consume_if_not_match>
    bool check_no_match(ValidatedChar ch)
    {
        return check_impl<assert_non_empty, false, consume_if_not_match>(ch);
    }

    template <bool assert_non_empty, bool consume_if_match, bool ...predicates(NativeByte ch)>
    bool check()
    {
        if constexpr(!assert_non_empty)
        {
            if (empty())
                return false;
        }

        if (!(predicates(front()) | ...))
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

    void advance_until(ValidatedChar ch)
    {
        while (!empty() && check_no_match<true, true>(ch))
            ;
    }

    __attribute__((always_inline))
    NativeByte const *save_str_begin()
    {
        return partial_line_segment.begin();
    }

    __attribute__((always_inline))
    string_view saved_str_end(NativeByte const *saved_begin)
    {
        return {saved_begin, partial_line_segment.begin()};
    }
};

struct SymbolString
{
    string_view symbol;
};

struct NumberString
{
    string_view number;
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
    AddressIndexField(
        std::variant<NativeInt, LiteralConstant, FutureReference> A,
        ValidatedRegisterIndex I,
        std::optional<ValidatedByte> F
    )
        : A(A), I(I), F(F)
    {}
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
using SymbolTable = std::unordered_map<string, ValueT, string_hash, std::equal_to<void>>;
using ResolvedSymbolTable = SymbolTable<ValidatedWord>;
using UnresolvedSymbolTable = SymbolTable<void *>;

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
    StdIstream &assembly;
    StdOstream &binary;
    string line;
    Cursor cursor;
    ValidatedWord location_counter;
    ResolvedSymbolTable symbol_table;
    UnresolvedSymbolTable unresolved_symbols;

    Result<void, Error>
    advance_location_counter(ValidatedPositiveWord by = one);

    void 
    add_symbol(string_view loc);

    void 
    add_symbol(string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_equ(string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_orig(string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_con(string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_alf(string_view loc, string_view str);
    
    Result<void, Error>
    assemble_end(string_view loc, ValidatedWord value);
    
    Result<void, Error>
    assemble_instruction(string_view loc, AddressIndexField AIF);
    // Value:
    // - true if end of input
    // - false otherwise
    Result<bool, Error>
    assemble_line();
public:
    Assembler(StdIstream &assembly, StdOstream &binary)
        : assembly(assembly), binary(binary), location_counter(zero)
    {}
    
    void
    assemble();
};

}
