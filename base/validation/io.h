#pragma once

#include <base/io.h>
#include <base/character_set.h>
#include <base/string.h>
#include <base/validation/v2.h>
#include <base/validation/types.h>
namespace mix
{

constexpr size_t max_char_len = []{
    size_t max_char_len = 0;
    for (Char ch_str : character_set) {
        max_char_len = std::max(max_char_len, ch_str.num_bytes());
    }
    return max_char_len;
}();
static_assert(max_char_len == 2);

// UTF-8 is a prefix code, and so we do not need lookahead.
// https://en.wikipedia.org/wiki/Self-synchronizing_code
struct CharToMixCharTransformer
{
    std::optional<char> char_buf; // No need for `std::array` since `max_char_len - 1 == 1`
    std::vector<ValidatedChar> mix_char_buf;
    size_t head = 0;
    Result<std::span<ValidatedChar>> recv()
    {
        size_t const old_head = 0;
        head = mix_char_buf.size();
        return Result<std::span<ValidatedChar>>::success(std::span(mix_char_buf.begin() + old_head, mix_char_buf.end()));
    }

    Result<std::optional<ValidatedChar>> recv_char()
    {
        using ResultType = Result<std::optional<ValidatedChar>>;
        if (head == mix_char_buf.size())
            return ResultType::success(std::nullopt);
        return ResultType::success(mix_char_buf[head++]);
    }

    Result<void> send(std::span<char> sp)
    {
        auto it = sp.begin();
        if (it == sp.end())
            return Result<void>::success();

        if (char_buf.has_value())
        {
            std::array<char, max_char_len> buf{char_buf.value(), *it};
            char_buf.reset();
        
            if (!utf8_to_mix_char(std::string_view(buf.begin(), buf.size())).transform_value([this](ValidatedChar ch){
                mix_char_buf.push_back(ch);
            }))
                return Result<void>::failure();
        
            it++;
        }
        
        while (it + (max_char_len - 1) < sp.end())
        {
            if (!utf8_to_mix_char(std::string_view(&*it, 1)).transform([this, &it](ValidatedChar ch){
                mix_char_buf.push_back(ch);
                it++;
            }, [this, &it](){
                return utf8_to_mix_char(std::string_view(&*it, 2)).transform_value([this, &it](ValidatedChar){
                    it += 2;
                });
            }))
                return Result<void>::failure();
        }

        if (it != sp.end())
        {
            utf8_to_mix_char(std::string_view(&*it, 1)).transform([this](ValidatedChar mix_char){
                mix_char_buf.push_back(mix_char);
            }, [this, it](){
                char_buf.emplace(*it);
            });
        }

        return Result<void>::success();
    }

    Result<void> send_char(char ch)
    {
        if (char_buf.has_value())
        {
            std::array<char, max_char_len> buf{char_buf.value(), ch};
            char_buf.reset();
            return utf8_to_mix_char(std::string_view(buf.begin(), buf.size())).transform_value([this](ValidatedChar mix_char){
                mix_char_buf.push_back(mix_char);
            });
        }
        else
        {
            return utf8_to_mix_char(std::string_view(&ch, 1)).transform([this](ValidatedChar mix_char){
                mix_char_buf.push_back(mix_char);
            }, [this, ch](){
                char_buf.emplace(ch);
                return Result<void>::success();
            });
        }
    }
};

static_assert(IsTransformer<char, ValidatedChar, CharToMixCharTransformer>);

struct MixByteToCharTransformer
{
    Result<std::span<char>> recv()
    {
        
    }
    Result<std::optional<char>> recv_char()
    {

    }
    Result<void> send(std::span<Byte> sp)
    {

    }
    Result<void> send_char(Byte ch)
    {
        
    }
};

static_assert(IsTransformer<Byte, char, MixByteToCharTransformer>);

}
