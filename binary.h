#include <base.h>

struct BinaryMagic 
{
    std::array<NativeByte, "MIX_MAGIC"sv.size()> contents;
};

enum HeaderRecordType
{
    program_header_size,
    program_header_offset,
    entry_point,
};

struct HeaderRecord
{
    HeaderRecordType type;
    Word<OwnershipKind::owns> value;
};

struct Header 
{
    
};

struct ProgramHeader
{

};

struct Binary
{
    BinaryMagic magic;
    Header header;
    ProgramHeader program_header;

};