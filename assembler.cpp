#include <assembler.h>
#include <base.h>

#include <array>
#include <limits>

template <bool is_peek, typename T>
__attribute__((always_inline))
static
T extract(std::istream &s)
{
    T t;
    __attribute__((unused)) std::optional<decltype(s.tellg())> saved_pos;
    if constexpr(is_peek)
        saved_pos = s.tellg();
    s >> t;
    if constexpr(is_peek)
        s.seekg(*saved_pos);
    return t;
}

template <bool is_peek, size_t n>
__attribute__((always_inline))
static
std::array<char, n> extract_buf(std::istream &s)
{
    std::array<char, n> buf;
    __attribute__((unused)) std::optional<decltype(s.tellg())> saved_pos;
    if constexpr(is_peek)
        saved_pos = s.tellg();
    s.get(buf.begin(), buf.size());
    if constexpr(is_peek)
        s.seekg(*saved_pos);
    return buf;
}

__attribute__((always_inline))
static
void skip_line(std::istream &s)
{
    s.ignore(std::numeric_limits<size_t>::max(), '\n');
}

Result<bool, Error>
Assembler::assemble_line()
{
    if (assembly.eof())
        return Result<bool, Error>::success(true);

    // first, check if this line is a comment
    auto const ch = assembly.peek();
    if (ch == '\n')
        goto unexpected_newline;

    if (ch == '*')
    {
        assembly.ignore(std::numeric_limits<size_t>::max(), '\n');
        if (assembly.fail())
            goto stdio_failed;
        else
            goto success;
    }

    // Now, this line is either an assembler directive or an assembler instruction
    // Both are of the form 
    // LOC <space> OP <space> ADDRESS COMMENTS
    // Where LOC can be empty
    // COMMENTS can be empty or otherwise must begin with a space
    if (ch != ' ')
    {
        auto const loc = extract<false, std::string>(assembly);
    }

success:
    return Result<bool, Error>::success(false);
stdio_failed:
    return Result<bool, Error>::failure(err_io);
unexpected_newline:
    g_logger << "Invalid MIX assembly\n";
    return Result<bool, Error>::failure(err_invalid_input);
}

void
Assembler::assemble()
{
    while (assemble_line())
        ;
}
