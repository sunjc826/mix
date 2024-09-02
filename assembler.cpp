#include <error.h>
#include <assembler.h>

#include <array>
#include <limits>
#include <string_view>

template <bool is_peek, typename T>
__attribute__((always_inline))
static
T 
extract(std::istream &s)
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
std::array<char, n> 
extract_buf(std::istream &s)
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
void 
skip_line(std::istream &s)
{
    s.ignore(std::numeric_limits<size_t>::max(), '\n');
}


Result<bool, Error>
Assembler::assemble_line()
{
    if (assembly.eof())
        return Result<bool, Error>::success(true);
    
    if (!std::getline(assembly, line))
        return Result<bool, Error>::failure(err_io);
    
    std::string_view line_view(line);
    if (line_view.empty())
    {
        if (assembly.eof())
            return Result<bool, Error>::success(true);
        else
        {
            g_logger << "Invalid MIX assembly: Empty line is not allowed, instead consider beginning with *\n";
            return Result<bool, Error>::failure(err_invalid_input);
        }
    }

    if (line_view.front() == '*')
        return Result<bool, Error>::success(false);

    // Now, this line is either an assembler directive or an assembler instruction
    // Both are of the form 
    // LOC <space> OP <space> ADDRESS COMMENTS
    // Where LOC can be empty
    // COMMENTS can be empty or otherwise must begin with a space
    std::string_view loc;
    {
        size_t const pos = line_view.find(' ');
        if (pos == std::string_view::npos)
        {
            g_logger << "OP and ADDRESS not found\n";
            return Result<bool, Error>::failure(err_invalid_input);
        }
        loc = line_view.substr(0, pos);
        line_view.remove_prefix(pos + 1);
    }

    std::string_view op;
    {
        size_t const pos = line_view.find(' ');
        if (pos == std::string_view::npos)
        {
            g_logger << "ADDRESS not found\n";
            return Result<bool, Error>::failure(err_invalid_input);
        }
        op = line_view.substr(0, pos);
        line_view.remove_prefix(pos + 1);
    }

    std::string_view address;
    {
        size_t const pos = line_view.find(' ');
        if (pos == std::string_view::npos)
        {
            address = line_view;
            line_view.remove_prefix(line_view.size());
        }
        else
        {
            address = line_view.substr(0, pos);
            line_view.remove_prefix(pos + 1);
        }
        std::string_view const address = line_view.substr(); 
    }

    __attribute__((unused)) std::string_view comments = line_view;

    if (op == "EQU")
    {
        // handle_equ(loc, address);
    }
    else if (op == "ORIG")
    {
        
    }
    else if (op == "CON")
    {

    }
    else if (op == "ALF")
    {

    }
    else if (op == "END")
    {
    
    }
    else 
    {
        
    }
    

    return Result<bool, Error>::success(false);
}

void
Assembler::assemble()
{
    while (assemble_line())
        ;
}
