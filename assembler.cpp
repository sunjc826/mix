#include <array>
#include <assembler.h>
#include <base.h>

template <typename T>
__attribute__((always_inline))
T extract(std::istream &s)
{
    T t;
    s >> t;
    return t;
}

template <size_t n>
__attribute__((always_inline))
std::array<char, n> extract_buf(std::istream &s)
{
    std::array<char, n> buf;
    s.get(buf.begin(), buf.size());
    return buf;
}

void
assemble(std::istream &assembly, std::ostream &binary)
{
    
}
