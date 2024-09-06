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

    __attribute__((always_inline))
    char front()
    {
        return partial_line_segment.front();
    }

    template <bool assert_non_empty, bool consume_if_match>
    bool check(char ch)
    {
        if constexpr(!assert_non_empty)
        {
            if (empty())
                return false;
        }

        if (front() != ch)
            return false;
        
        if constexpr(consume_if_match)
            advance();

        return true;
    }

    template <bool assert_non_empty, bool consume_if_match, int ctype_predicate(int ch)>
    bool check()
    {
        if constexpr(!assert_non_empty)
        {
            if (empty())
                return false;
        }

        if (!ctype_predicate(front()))
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

    ptrdiff_t extent_of_parsing() const
    {
        return partial_line_segment.begin() - full_line_segment.begin();
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

template <typename ValueT>
using SymbolTable = std::unordered_map<std::string, ValueT, string_hash, std::equal_to<void>>;
using ResolvedSymbolTable = SymbolTable<NativeInt>;
using UnresolvedSymbolTable = SymbolTable<void *>;

struct ExpressionParser
{
    Cursor &cursor;
    ResolvedSymbolTable const &symbol_table;
    UnresolvedSymbolTable const &unresolved_symbols;
    ExpressionParser(Cursor &cursor, ResolvedSymbolTable const &symbol_table, UnresolvedSymbolTable const &unresolved_symbols)
        : cursor(cursor), symbol_table(symbol_table), unresolved_symbols(unresolved_symbols)
    {}

    std::variant<SymbolString, NumberString, EmptyString>
    next_symbol_or_number();

    // An atomic expression is defined as one of
    // (1) number
    // (2) defined symbol
    // (3) asterisk
    // `try_...` refers to the possibility of having no atomic expression, 
    // in that case, std::nullopt is the result.
    Result<std::optional<NativeInt>, Error>
    try_parse_atomic_expression();

    // An <expression> is defined as one of
    // (1) <atomic expression>
    // (2) <atomic expression> <binary op> <expression>
    // where <binary op> is one of
    // +, -, *, /, //, :
    // There are no associativity rules and the expression is evaluated from left to right.
    Result<std::optional<NativeInt>, Error>
    try_parse_expression();

    // A <literal constant> is defined as
    // = <W value length -lt 10> =
    // where <W value length -lt 10> is defined as
    // a <W value> whose length is less than 10.
    Result<std::optional<LiteralConstant>, Error>
    try_parse_literal_constant();

    std::optional<FutureReference>
    try_parse_future_reference();

    Result<std::variant<NativeInt, LiteralConstant, FutureReference>, Error>
    parse_A_part();

    Result<ValidatedRegisterIndex, Error>
    parse_index_part();

    Result<VariantWithDefault_t<ValidatedByte>, Error>
    parse_F_part();

    Result<void, Error>
    parse_instruction_address();

    Result<NativeInt, Error>
    parse_W_value();
};

class Assembler
{
    std::istream &assembly;
    std::ostream &binary;
    std::string line;
    
    void assemble_equ(std::string_view loc, std::string_view address);
    void assemble_orig(std::string_view loc, std::string_view address);
    void assemble_con(std::string_view loc, std::string_view address);
    void assemble_alf(std::string_view loc, std::string_view address);
    void assemble_end(std::string_view loc, std::string_view address);
    // Value:
    // - true if end of input
    // - false otherwise
    Result<bool, Error>
    assemble_line();
public:
    Assembler(std::istream &assembly, std::ostream &binary)
        : assembly(assembly), binary(binary)
    {}
    
    void
    assemble();
};
