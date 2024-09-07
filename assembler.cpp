#include "assembler.defn.h"
#include "base.h"
#include <algorithm>
#include <assembler.h>
#include <register.h>
#include <error.h>

#include <array>
#include <limits>
#include <string_view>
#include <variant>

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

ValidatedWord
ExpressionParser::evaluate(ValidatedWord lhs, BinaryOp op, ValidatedWord rhs)
{

}

std::optional<BinaryOp>
ExpressionParser::try_parse_binary_op()
{
    if (cursor.empty())
        return std::nullopt;

    std::optional<BinaryOp> op;
    switch (cursor.front())
    {
    case '+':
        op = BinaryOp::add; cursor.advance();
        break;
    case '-':
        op = BinaryOp::subtract; cursor.advance();
        break;
    case '*':
        op = BinaryOp::multiply; cursor.advance();
        break;
    case '/':
        op = BinaryOp::divide; cursor.advance();
        if (cursor.check<false, true>('/'))
        {
            op = BinaryOp::double_slash; cursor.advance();
        }
        break;
    case ':':
        op = BinaryOp::colon; cursor.advance();
        break;
    }
    return op;
}

std::variant<SymbolString, NumberString, EmptyString>
ExpressionParser::try_parse_symbol_or_number()
{
    char const *begin = cursor.save_str_begin();
    while (!cursor.check<false, true, std::isdigit, std::isupper>())
        ;
    std::string_view const sv = cursor.saved_str_end(begin);
    if (sv.empty())
        return EmptyString{};
    
    if (std::find_if(sv.begin(), sv.end(), [](char ch){ return std::isupper(ch); }) != sv.end())
        return SymbolString{sv};
    
    return NumberString{sv};
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
    
    auto const symbol_or_number = try_parse_symbol_or_number();
    
    if (auto *symbol = std::get_if<SymbolString>(&symbol_or_number))
    {
        return FutureReference{*symbol};
    }
    else 
    {
        cursor = saved_cursor;
        return std::nullopt;
    }
}

Result<std::optional<ValidatedWord>, Error>
ExpressionParser::try_parse_atomic_expression()
{
    using ResultType = Result<std::optional<ValidatedWord>, Error>;
    auto const symbol_or_number = try_parse_symbol_or_number();
    if (auto *number = std::get_if<NumberString>(&symbol_or_number))
    {
        auto const it = std::find_if_not(number->number.begin(), number->number.end(), [](char ch){return ch == '0';});
        if (it == number->number.end())
            return ResultType::success(0);
        std::optional<ValidatedWord> const value = ValidatedWord::constructor(std::strtol(it, NULL, 10));
        if (!value)
        {
            g_logger << "Integer literal does not fit in a MIX word\n";
            return ResultType::failure(err_overflow);
        }
        return ResultType::success(*value);
    }
    else if (auto *symbol = std::get_if<SymbolString>(&symbol_or_number))
    {
        auto const it = symbol_table.find(symbol->symbol);
        if (it == symbol_table.end())
        {
            g_logger << "Symbol not found in symbol table\n";
            return ResultType::failure(err_invalid_input);
        }
        return ResultType::success(it->second);
    }
    else // EmptyString
    {
        return ResultType::success(std::nullopt);
    }
}

Result<std::optional<ValidatedWord>, Error>
ExpressionParser::try_parse_expression()
{
    using ResultType = Result<std::optional<ValidatedWord>, Error>;
    
    auto const atomic_expression_result = try_parse_atomic_expression();
    if (!atomic_expression_result)
        return ResultType::failure(err_invalid_input);

    std::optional<ValidatedWord> const atomic_expression = atomic_expression_result;
    if (!atomic_expression)
        return ResultType::success(std::nullopt);

    ValidatedWord value = *atomic_expression;

    while (true)
    {
        std::optional<BinaryOp> const binary_op = try_parse_binary_op();
        if (!binary_op)
            break;
        
        auto const atomic_expression_result = try_parse_atomic_expression();
        if (!atomic_expression_result)
            return ResultType::failure(err_invalid_input);

        std::optional<ValidatedWord> const atomic_expression = atomic_expression_result;
        if (!atomic_expression)
            return ResultType::failure(err_invalid_input);

        value = evaluate(value, *binary_op, *atomic_expression);
    }
    
    return ResultType::success(value);
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
        return ResultType::success(*literal_constant);

    std::optional<FutureReference> const future_reference = try_parse_future_reference();
    if (future_reference)
        return ResultType::success(*future_reference);
    
    auto const expression_result = try_parse_expression();  
    if (!expression_result)
        return ResultType::failure(err_invalid_input);

    std::optional<NativeInt> const expression = expression_result;
    if (expression)
        return ResultType::success(*expression);

    return ResultType::success(0);
}

Result<ValidatedRegisterIndex, Error>
ExpressionParser::parse_index_part()
{
    if (!cursor.check<false, true>(','))
        return Result<ValidatedRegisterIndex, Error>::success(0);
    
    auto const expression_result = try_parse_expression();
    if (!expression_result)
        return Result<ValidatedRegisterIndex, Error>::failure(err_invalid_input);
    NativeInt const expression = expression_result;

    std::optional<ValidatedRegisterIndex> const index = ValidatedRegisterIndex::constructor(expression);
    if (!index)
        return Result<ValidatedRegisterIndex, Error>::failure(err_invalid_input);
    return Result<ValidatedRegisterIndex, Error>::success(*index);
}

Result<std::optional<ValidatedByte>, Error>
ExpressionParser::parse_F_part()
{
    if (!cursor.check<false, true>('('))
        return Result<std::optional<ValidatedByte>, Error>::success(std::nullopt);

    auto const expression_result = try_parse_expression();
    if (!expression_result)
        return Result<std::optional<ValidatedByte>, Error>::failure(err_invalid_input);
    NativeInt const expression = expression_result;
    
    std::optional<ValidatedByte> const F_part = ValidatedByte::constructor(expression);
    if (!F_part)
        return Result<std::optional<ValidatedByte>, Error>::failure(err_invalid_input);

    if (!cursor.check<false, true>(')'))
    {
        g_logger << "Missing right bracket of F-part\n";
        return Result<std::optional<ValidatedByte>, Error>::failure(err_invalid_input);
    }

    return Result<std::optional<ValidatedByte>, Error>::success(*F_part);
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
        auto const expression_result = try_parse_expression();
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
