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

struct FutureReference
{
    std::string_view symbol;
};

struct LiteralConstant
{
    NativeInt value;
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

struct ExpressionParser
{
    Cursor &cursor;
    std::unordered_map<std::string, NativeInt> const &symbol_table;
    ExpressionParser(Cursor &cursor, std::unordered_map<std::string, NativeInt> const &symbol_table)
        : cursor(cursor), symbol_table(symbol_table)
    {}

    std::variant<SymbolString, NumberString, EmptyString>
    next_symbol_or_number();

    Result<NativeInt, Error>
    parse_atomic_expression();

    Result<NativeInt, Error>
    parse_expression();

    Result<std::optional<LiteralConstant>, Error>
    try_parse_literal_constant();

    std::optional<FutureReference>
    try_parse_future_reference();

    Result<std::variant<NativeInt, LiteralConstant, FutureReference>, Error>
    parse_A_part();

    Result<NativeByte, Error>
    parse_index_part();

    Result<std::optional<NativeByte>, Error>
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
