#pragma once
#include <base.h>
#include <error.h>

#include <istream>
#include <unordered_map>

struct BufferedIStream
{
    std::istream input;
};

struct ExpressionParser
{
    std::string_view sv;
    std::unordered_map<std::string, NativeInt> const &symbol_table;
    ExpressionParser(std::string_view expr, std::unordered_map<std::string, NativeInt> const &symbol_table)
        : sv(expr), symbol_table(symbol_table)
    {}

    // Precondition: sv non-empty
    char getchar();

    void
    parse_number();

    void
    parse_atomic_expression();

    Result<NativeInt, Error>
    parse_expression();

    Result<NativeInt, Error>
    parse_A_part();

    Result<NativeByte, Error>
    parse_index_part();

    Result<NativeByte, Error>
    parse_F_part();

    Result<NativeInt, Error>
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
