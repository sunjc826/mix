#include "assembler.defn.h"
#include <assembler.h>
#include <register.h>
#include <error.h>

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

std::string_view
ExpressionParser::next_symbol_or_number()
{
    SavedCursorStringState state = cursor.save_str_begin();
    while (!cursor.check<false, true, std::isalnum>())
        ;
    return cursor.saved_str_end(state);
}

Result<std::optional<LiteralConstant>, Error>
ExpressionParser::try_parse_literal_constant()
{
    if (!cursor.check<false, true>('='))
        return Result<std::optional<LiteralConstant>, Error>::success(std::nullopt);
    size_t const saved_position = cursor.column_number;
    auto const W_value_result = parse_W_value();
    size_t const current_position = cursor.column_number;
    if (current_position - saved_position >= 10)
    {
        g_logger << "W value too long\n";
        return Result<std::optional<LiteralConstant>, Error>::failure(err_invalid_input);
    }

    if (!cursor.check<false, true>('='))
    {
        g_logger << "Expected closing =\n";
        return Result<std::optional<LiteralConstant>, Error>::failure(err_invalid_input);
    }

    NativeInt const W_value = W_value_result;
    return Result<std::optional<LiteralConstant>, Error>::success(W_value);
}

std::optional<FutureReference>
ExpressionParser::try_parse_future_reference()
{
    Cursor const saved_cursor = cursor;
    
    std::string_view const symbol_or_number = next_symbol_or_number();
    
    auto it = std::find_if(symbol_or_number.begin(), symbol_or_number.end(), [](char ch){
        return std::isalpha(ch);
    });
    if (it == symbol_or_number.end())
    {
        cursor = saved_cursor;
        return std::nullopt;
    }

    return FutureReference{.symbol = symbol_or_number};
}

Result<NativeInt, Error>
parse_expression()
{

}

Result<std::variant<NativeInt, LiteralConstant, FutureReference>, Error>
ExpressionParser::parse_A_part()
{
    using ResultType = Result<std::variant<NativeInt, LiteralConstant, FutureReference>, Error>;
    auto const literal_constant_result = try_parse_literal_constant();
    if (!literal_constant_result)
        return ResultType::failure(err_invalid_input);
    std::optional<LiteralConstant> const literal_constant = literal_constant_result;
    if (literal_constant)
    {
        return ResultType::success(*literal_constant);
    }

    auto const future_reference_result = try_parse_future_reference();
    

    parse_expression();    
}

Result<NativeByte, Error>
ExpressionParser::parse_index_part()
{
    if (!cursor.check<false, true>(','))
        return Result<NativeByte, Error>::success(0);
    
    auto const expression_result = parse_expression();
    if (!expression_result)
        return Result<NativeByte, Error>::failure(err_invalid_input);
    NativeInt const expression = expression_result;

    auto const index_result = mix_int_to_mix_byte(expression);
    if (!index_result)
        return Result<NativeByte, Error>::failure(err_invalid_input);
    NativeByte const index = index_result;

    return Result<NativeByte, Error>::success(index);
}

Result<std::optional<NativeByte>, Error>
ExpressionParser::parse_F_part()
{
    if (!cursor.check<false, true>('('))
        return Result<std::optional<NativeByte>, Error>::success(std::nullopt);

    auto const expression_result = parse_expression();
    if (!expression_result)
        return Result<std::optional<NativeByte>, Error>::failure(err_invalid_input);
    NativeInt const expression = expression_result;
    
    auto const F_part_result = mix_int_to_mix_byte(expression);
    if (!F_part_result)
        return Result<std::optional<NativeByte>, Error>::failure(err_invalid_input);

    if (!cursor.check<false, true>(')'))
    {
        g_logger << "Missing right bracket of F-part\n";
        return Result<std::optional<NativeByte>, Error>::failure(err_invalid_input);
    }

    NativeByte const F_part = F_part_result;
    return Result<std::optional<NativeByte>, Error>::success(F_part);
}

Result<NativeInt, Error>
ExpressionParser::parse_instruction_address()
{

}

Result<NativeInt, Error> 
ExpressionParser::parse_W_value()
{
    Register<true, bytes_in_word> reg;
    Word<OwnershipKind::owns> word;
    reg.store(word);
    do
    {
        auto const expression_result = parse_expression();
        if (!expression_result)
            return Result<NativeInt, Error>::failure(err_invalid_input);
        NativeInt const expression = expression_result;
        auto const F_part_result = parse_F_part();
        if (!F_part_result)
            return Result<NativeInt, Error>::failure(err_invalid_input);
        std::optional<NativeByte> const F_part = F_part_result;
        
        DeferredValue<SliceMutable> slice;
        if (F_part)
        {
            FieldSpec const field_spec = FieldSpec::from_byte(*F_part);
            bool const is_overflow = reg.load<false>(expression);
            if (is_overflow)
                return Result<NativeInt, Error>::failure(err_overflow);
            slice.construct(word, field_spec);
        }
        else
            slice.construct(word);
        reg.store(slice);
    }
    while(!cursor.check<false, true>(','));

    NativeInt const W_value = word.native_value();
    return Result<NativeInt, Error>::success(W_value);
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
