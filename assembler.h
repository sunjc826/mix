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
public:
    Assembler(std::istream &assembly, std::ostream &binary)
        : assembly(assembly), binary(binary)
    {}

    // Value:
    // - true if end of input
    // - false otherwise
    Result<bool, Error>
    assemble_line();
    
    void
    assemble();
};
