#include <base.h>
#include <stdexcept>

struct BinaryMagic 
{
    std::array<NativeByte, "MIX_MAGIC"sv.size()> contents;
};

enum HeaderRecordType : NativeByte
{
    hr_program_header_size,
    hr_program_header_offset,
    hr_entry_point,
    hr_max,
};

struct HeaderRecord
{
    HeaderRecordType type;
    Word<OwnershipKind::owns> word;
};

struct Header 
{
    std::array<std::optional<HeaderRecord>, hr_max> records;

    void check() const
    {
        for (size_t i = 0; i < records.size(); i++)
        {
            if (records[i])
            {
                if (records[i]->type != i)
                    throw std::runtime_error("expected type to be equal to index");
            }
        }
    }
};

enum ProgramHeaderRecordType : NativeByte
{
    phr_load,
    phr_max,
};

struct ProgramHeaderRecord
{
    ProgramHeaderRecordType type;
    Word<OwnershipKind::owns> offset;
    Word<OwnershipKind::owns> size;  
};

struct ProgramHeader
{
    std::vector<ProgramHeaderRecord> records;
};

struct Binary
{
    BinaryMagic magic;
    Header header;
    ProgramHeader program_header;
    NativeByte padding[main_memory_size];
};