#include "base/string.h"
#include "base/types.decl.h"
#include "base/validated_int.defn.h"
#include <base/error.h>
#include <base/io.h>
#include <binary/assembler.h>
#include <vm/register.h>

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>
#include <variant>
namespace mix
{

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

Result<ValidatedWord, Error>
ExpressionParser::evaluate(ValidatedWord lhs, BinaryOp op, ValidatedWord rhs)
{
    using ResultType = Result<ValidatedWord, Error>;
    NativeInt result;
    switch (op)
    {
    case BinaryOp::add:
        // TODO: Add warning on overflow.
        result = (lhs + rhs) % lut[numerical_bytes_in_word];
        break;
    case BinaryOp::subtract:
        result = (lhs + rhs) % lut[numerical_bytes_in_word];
        break;
    case BinaryOp::multiply:
        result = (lhs * rhs) % lut[numerical_bytes_in_word];
        break;
    case BinaryOp::divide:
        result = lhs / rhs;
        // In MIX, overflow of the quotient is undefined behavior.
        // This implementation decides to give an error.
        if (std::abs(result) >= lut[numerical_bytes_in_word])
            return ResultType::failure(err_overflow);
        break;
    case BinaryOp::double_slash:
        result = (lhs * lut[numerical_bytes_in_word]) / rhs;
        if (std::abs(result) >= lut[numerical_bytes_in_word])
            return ResultType::failure(err_overflow);
        break;
    case BinaryOp::colon:
        // The : operation is defined as
        // LDA AA; MUL =8=; SLAX 5; ADD BB; STA CC
        // Both `MUL =8=; SLAX 5` and `ADD BB` have the effect of modulo `lut[numerical_bytes_in_word]`
        result = (lhs * 8 + rhs) % lut[numerical_bytes_in_word];
        break;
    }
    auto eval_result = ValidatedWord::constructor(result);
    if (!eval_result)
        return ResultType::failure(err_overflow);
    return ResultType::success(eval_result.value());
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
        if (cursor.check<false, true>(ascii_to_mix_char('/')))
            op = BinaryOp::double_slash;
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
    NativeByte const *begin = cursor.save_str_begin();
    while (!cursor.check<false, true, isalnum>())
        ;
    string_view const sv = cursor.saved_str_end(begin);
    if (sv.empty())
        return EmptyString{};
    
    if (std::find_if(sv.begin(), sv.end(), [](char ch){ return std::isupper(ch); }) != sv.end())
        return SymbolString{sv};
    
    return NumberString{sv};
}

Result<std::optional<LiteralConstant>, Error>
ExpressionParser::try_parse_literal_constant()
{
    if (!cursor.check<false, true>(ascii_to_mix_char('=')))
        return Result<std::optional<LiteralConstant>, Error>::success(std::nullopt);
    size_t const saved_position = cursor.column_number;
    auto const W_value_result = parse_W_value();
    size_t const current_position = cursor.column_number;
    if (current_position - saved_position >= 10)
    {
        g_logger << "W value too long\n";
        return Result<std::optional<LiteralConstant>, Error>::failure(err_invalid_input);
    }

    if (!cursor.check<false, true>(ascii_to_mix_char('=')))
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
            return ResultType::success(zero);
        Result<ValidatedWord, Error> const conversion_result = strtoword(it);
        if (!conversion_result)
        {
            g_logger << "Integer literal does not fit in a MIX word\n";
            return ResultType::failure(err_overflow);
        }
        return ResultType::success(conversion_result.value());
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

    std::optional<ValidatedWord> const atomic_expression = atomic_expression_result.value();
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

        std::optional<ValidatedWord> const atomic_expression = atomic_expression_result.value();
        if (!atomic_expression)
            return ResultType::failure(err_invalid_input);

        auto const eval_result = evaluate(value, *binary_op, *atomic_expression);
        if (!eval_result)
            return ResultType::failure(err_invalid_input);
        value = eval_result.value();
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

    std::optional<LiteralConstant> const literal_constant = literal_constant_result.value();
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
ExpressionParser::parse_I_part()
{
    if (!cursor.check<false, true>(ascii_to_mix_char(',')))
        return Result<ValidatedRegisterIndex, Error>::success(zero);
    
    auto const expression_result = try_parse_expression();
    if (!expression_result)
        return Result<ValidatedRegisterIndex, Error>::failure(err_invalid_input);
    NativeInt const expression = expression_result;

    auto const I_value_result = ValidatedRegisterIndex::constructor(expression);
    if (!I_value_result)
        return Result<ValidatedRegisterIndex, Error>::failure(err_invalid_input);
    
    ValidatedRegisterIndex const I_value = I_value_result.value();
    return Result<ValidatedRegisterIndex, Error>::success(I_value);
}

Result<std::optional<ValidatedByte>, Error>
ExpressionParser::parse_F_part()
{
    using ResultType = Result<std::optional<ValidatedByte>, Error>;
    if (!cursor.check<false, true>(ascii_to_mix_char('(')))
        return ResultType::success(std::nullopt);

    auto const expression_result = try_parse_expression();
    if (!expression_result)
        return ResultType::failure(err_invalid_input);
    NativeInt const expression = expression_result;
    
    auto const F_part_result = ValidatedByte::constructor(expression);
    if (!F_part_result)
        return ResultType::failure(err_invalid_input);

    if (!cursor.check<false, true>(ascii_to_mix_char(')')))
    {
        g_logger << "Missing right bracket of F-part\n";
        return ResultType::failure(err_invalid_input);
    }

    return ResultType::success(F_part_result.value());
}

Result<AddressIndexField, Error>
ExpressionParser::parse_AIF()
{
    using ResultType = Result<AddressIndexField, Error>;
    auto const A_part_result = parse_A_part();
    if (!A_part_result)
        return ResultType::failure(err_invalid_input);
    auto const I_part_result = parse_I_part();
    if (!I_part_result)
        return ResultType::failure(err_invalid_input);
    auto const F_part_result = parse_F_part();
    if (!F_part_result)
        return ResultType::failure(err_invalid_input);
    return ResultType::success(A_part_result.value(), I_part_result.value(), F_part_result.value());
}

Result<ValidatedWord, Error> 
ExpressionParser::parse_W_value()
{
    using ResultType = Result<ValidatedWord, Error>;
    Register<true, bytes_in_word> reg;
    Word<OwnershipKind::owns> word;
    reg.store(word);
    do
    {
        auto const expression_result = try_parse_expression();
        if (!expression_result)
            return ResultType::failure(err_invalid_input);
        NativeInt const expression = expression_result;
        auto const F_part_result = parse_F_part();
        if (!F_part_result)
            return ResultType::failure(err_invalid_input);
        std::optional<NativeByte> const F_part = F_part_result;
        
        DeferredValue<SliceMutable> slice;
        if (F_part)
        {
            FieldSpec const field_spec = FieldSpec::from_byte(*F_part);
            bool const is_overflow = reg.load<false>(expression);
            if (is_overflow)
                return ResultType::failure(err_overflow);
            slice.construct(word, field_spec);
        }
        else
            slice.construct(word);
        reg.store(slice);
    }
    while(!cursor.check<false, true>(ascii_to_mix_char(',')));

    auto const W_value_result = ValidatedWord::constructor(word.native_value());
    if (!W_value_result)
    {
        g_logger << "Unexpected overflow of W_value\n";
        return ResultType::failure(err_internal_logic);
    }

    ValidatedWord const W_value = W_value_result.value();
    return ResultType::success(W_value);
}

void
Assembler::add_symbol(string_view loc)
{
    symbol_table.insert({ string(loc), location_counter });
}

void
Assembler::add_symbol(string_view loc, ValidatedWord value)
{
    symbol_table.insert({ string(loc), value });
}

Result<void, Error>
Assembler::assemble_equ(string_view loc, ValidatedWord value)
{
    using ResultType = Result<void, Error>;
    if (loc.empty())
    {
        g_logger << "EQU directive must have an associated symbol\n";
        return ResultType::failure(err_invalid_input);
    }

    add_symbol(loc, value);
    return ResultType::success();
}

Result<void, Error>
Assembler::assemble_orig(string_view loc, ValidatedWord value)
{
    using ResultType = Result<void, Error>;
    if (!loc.empty())
        add_symbol(loc);
    
    
}

Result<void, Error>
Assembler::assemble_con(string_view loc, ValidatedWord value)
{
    using ResultType = Result<void, Error>;
    if (!loc.empty())
        add_symbol(loc);
   
    std::array<Byte, bytes_in_word> const bytes = as_bytes(value);
    binary << bytes[0].sign;
    for (size_t i = 1; i < bytes.size(); i++)
        binary << bytes[i].byte;
    auto increment_result = location_counter.increment();
    if (!increment_result)
        return ResultType::failure(err_overflow);
    location_counter = increment_result.value();
    return ResultType::success();
}

Result<void, Error>
Assembler::assemble_alf(string_view loc, string_view str)
{
    if (!loc.empty())
        add_symbol(loc);

    binary << s_plus << str;
}

void
Assembler::assemble_end(string_view loc, ValidatedWord value)
{

}

Result<bool, Error>
Assembler::assemble_line()
{
    using ResultType = Result<bool, Error>;
    if (assembly.eof())
        return ResultType::success(true);
    
    if (!std::getline(assembly, line))
        return ResultType::failure(err_io);
    
    cursor.process_next_line(line);
    
    if (cursor.empty())
    {
        if (assembly.eof())
            return ResultType::success(true);
        else
        {
            g_logger << "Invalid MIX assembly: Empty line is not allowed, instead consider beginning with *\n";
            return ResultType::failure(err_invalid_input);
        }
    }

    if (cursor.check<true, false>(ascii_to_mix_char('*')))
        return ResultType::success(false);

    // Now, this line is either an assembler directive or an assembler instruction
    // Both are of the form 
    // LOC <space> OP <space> ADDRESS COMMENTS
    // Where LOC can be empty
    // COMMENTS can be empty or otherwise must begin with a space
    string_view loc;
    {
        auto begin = cursor.save_str_begin();
        cursor.advance_until(ascii_to_mix_char(' '));
        if (cursor.empty())
        {
            g_logger << "OP and ADDRESS not found\n";
            return ResultType::failure(err_invalid_input);
        }
        loc = cursor.saved_str_end(begin);
        cursor.advance();
    }

    string_view op;
    {
        auto begin = cursor.save_str_begin();
        cursor.advance_until(ascii_to_mix_char(' '));
        if (cursor.empty())
        {
            g_logger << "ADDRESS not found\n";
            return ResultType::failure(err_invalid_input);
        }
        loc = cursor.saved_str_end(begin);
        cursor.advance();
    }

    string_view address;
    {
        ExpressionParser expression_parser(cursor, symbol_table, unresolved_symbols);
        
        auto begin = cursor.save_str_begin();
        cursor.advance_until(ascii_to_mix_char(' '));
        address = cursor.saved_str_end(begin);
        if (!cursor.empty())
            cursor.advance();
    }

    if (!loc.empty())
    {
        if (symbol_table.find(loc) != symbol_table.end())
        {
            g_logger << "Symbol already exists\n";
            return ResultType::failure(err_duplicate_symbol);
        }
    }

#define EXPRESSION_PARSER() ExpressionParser(cursor, symbol_table, unresolved_symbols)    
    
    if (streq(op, "EQU")) // ADDRESS is W value
    {
        auto const W_value_result = EXPRESSION_PARSER().parse_W_value();
        if (!W_value_result)
        {
            return ResultType::failure(err_invalid_input);
        }
       
        ValidatedWord const W_value = W_value_result.value();
        assemble_equ(loc, W_value);
    }
    else if (streq(op, "ORIG")) // ADDRESS is W value
    {
        auto const W_value_result = EXPRESSION_PARSER().parse_W_value();
        if (!W_value_result)
        {
            return ResultType::failure(err_invalid_input);
        }

        ValidatedWord const W_value = W_value_result.value();
        assemble_orig(loc, W_value);
    }
    else if (streq(op, "CON")) // ADDRESS is W value
    {
        auto const W_value_result = EXPRESSION_PARSER().parse_W_value();
        if (!W_value_result)
        {
            return ResultType::failure(err_invalid_input);
        }

        ValidatedWord const W_value = W_value_result.value();
        assemble_con(loc, W_value);
    }
    else if (streq(op, "ALF")) // ADDRESS is 5 characters
    {
        if (cursor.length() < 5)
        {
            return Result<bool, Error>::failure(err_invalid_input);
        }

        auto begin = cursor.save_str_begin();
        cursor.advance_by(5);
        auto str = cursor.saved_str_end(begin);
        assemble_alf(loc, str);
    }
    else if (streq(op, "END")) // ADDRESS is W value
    {
        auto const W_value_result = EXPRESSION_PARSER().parse_W_value();
        if (!W_value_result)
        {
            return ResultType::failure(err_invalid_input);
        }

        ValidatedWord const W_value = W_value_result.value();
        assemble_end(loc, W_value);
    }
    else 
    {
        auto const AIF_result = EXPRESSION_PARSER().parse_AIF();
        if (!AIF_result)
        {
            return ResultType::failure(err_invalid_input);
        }

        AddressIndexField const AIF = AIF_result.value();
        assemble_instruction(loc, AIF);
    }
#undef EXPRESSION_PARSER
    __attribute__((unused)) string_view const comments = cursor.partial_line_segment;

    return Result<bool, Error>::success(false);
}

void
Assembler::assemble()
{
    while (assemble_line())
        ;
}

}
