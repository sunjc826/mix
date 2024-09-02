#pragma once
#include <base.h>
#include <error.h>

#include <istream>

struct BufferedIStream
{
    std::istream input;
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
