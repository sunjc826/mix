#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/validation/v2.h>
#include <limits>

namespace mix
{
    using string = std::basic_string<NativeByte>;
    using string_view = std::basic_string_view<NativeByte>;

    inline constexpr 
    Result<ValidatedChar, void>
    utf8_to_mix_char(std::string_view utf8)
    {
        auto it = std::find(character_set.begin(), character_set.end(), Char{utf8});
        return ValidatedChar::constructor(it - character_set.begin());
    }

    // consteval because it is highly unlikely we want to specify a runtime ascii char
    // The call to .value() also ensures compile-time validation.
    inline consteval 
    ValidatedChar 
    ascii_to_mix_char(char ascii)
    {
        return utf8_to_mix_char(std::string_view(&ascii, 1L)).value();
    }

    inline bool 
    streq(string_view mix_str, std::string_view ascii_str)
    {
        if (mix_str.size() != ascii_str.size())
            return false;
        
        for (size_t i = 0; i < mix_str.size(); i++)
            if (utf8_to_mix_char(ascii_str.substr(i, 1)).value() != mix_str[i])
                return false;
        return true;
    }

    inline 
    Result<ValidatedWord, Error> 
    strtoword(NativeByte const *ptr)
    {
        using ResultType = Result<ValidatedWord, Error>;
        
        NativeInt result = 0;
        
        if (!isdigit(*ptr))
            return ResultType::failure(err_invalid_input);
        
        do
        {
            if (result >= std::numeric_limits<NativeInt>::max() / 10)
                return ResultType::failure(err_overflow);

            result = result * 10;
            result += *ptr - ascii_to_mix_char('0');
        }
        while (isdigit(*ptr));

        auto const word_result = ValidatedWord::constructor(result);
        if (!word_result)
            return ResultType::failure(err_overflow);
        return ResultType::success(word_result.value());
    }

};
