#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/validation/v2.h>
#include <limits>
#include <utility>

namespace mix
{
    using string = std::basic_string<NativeByte>;
    using string_view = std::basic_string_view<NativeByte>;
    
    namespace details 
    {

    template <typename Fn, size_t ...Is>
    consteval
    decltype(auto) dispatch(Fn fn, std::index_sequence<Is...>)
    {
        return std::array{ fn(Is)... };
    }

    }

    std::unordered_map<std::string_view, ValidatedChar> const char_lut{[](){
        std::unordered_map<std::string_view, ValidatedChar> char_lut;
        static std::array<ValidatedChar, character_set.size()> constexpr arr = details::dispatch([](NativeByte i){
            return ValidatedChar::constructor(i).value();
        }, std::make_index_sequence<character_set.size()>());
        for (size_t i = 0; i < arr.size(); i++) {
            char_lut.emplace(character_set[i].utf8_value, arr[i]);
        }
        return char_lut;
    }()};

    inline
    Result<ValidatedChar, void>
    utf8_to_mix_char(std::string_view utf8)
    {
        auto it = char_lut.find(utf8);
        if (it == char_lut.end())
            return Result<ValidatedChar, void>::failure();
        return Result<ValidatedChar, void>::success(it->second);
    }

    inline consteval
    Result<ValidatedChar, void>
    utf8_to_mix_char_compile_time(std::string_view utf8)
    {
        auto it = std::find(character_set.begin(), character_set.end(), Char{utf8});
        return ValidatedChar::constructor(it - character_set.begin());
    }

    inline consteval
    ValidatedChar 
    ascii_to_mix_char(char ascii)
    {
        return utf8_to_mix_char_compile_time(std::string_view(&ascii, 1L)).value();
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

}
