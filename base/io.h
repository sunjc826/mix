#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/result.h>

#include <istream>
#include <concepts>
#include <optional>
#include <string>

namespace mix
{

    template <typename CharT, typename OstreamT>
    concept IsOstream = requires(OstreamT os)
    {
        {
            os.write(std::declval<std::span<CharT>>())
        } -> std::same_as<Result<void>>;

        {
            os.putchar(std::declval<CharT>())
        } -> std::same_as<Result<void>>;
    };

    template <typename CharT, typename IstreamT>
    concept IsIstream = requires(IstreamT is, size_t buf_size)
    {
        {
            is.read(buf_size)
        } -> std::same_as<Result<std::span<CharT>>>;
        {
            is.readsome(buf_size)
        } -> std::same_as<Result<std::span<CharT>>>;

        {
            is.getchar()
        } -> std::same_as<Result<std::optional<CharT>>>;
    };

    class StdIstream
    {
        std::istream &is;
        std::string s;
    public:
        StdIstream(std::istream &is)
            : is(is)
        {}

        Result<std::span<char>> read(size_t buf_size)
        {
            s.resize(buf_size);
            is.readsome(s.data(), buf_size);
            if (!is)
                return Result<std::span<char>>::failure();
            return Result<std::span<char>>::success(s);
        }


        Result<std::span<char>> readsome(size_t buf_size)
        {
            s.resize(buf_size);
            s.resize(is.readsome(s.data(), buf_size));
            if (!is)
                return Result<std::span<char>>::failure();
            return Result<std::span<char>>::success(s);
        }

        Result<std::optional<char>> getchar()
        {
            if (!is)
                return Result<std::optional<char>>::failure();
            int ch = is.get();
            if (ch == std::char_traits<char>::eof())
                return Result<std::optional<char>>::success(std::nullopt);
            return Result<std::optional<char>>::success(ch);
        }
    };
    static_assert(IsIstream<char, StdIstream>);

    class StdOstream
    {
        std::ostream &os;
    public:
        StdOstream(std::ostream &os)
            : os(os)
        {}

        Result<void> write(std::span<char> sv)
        {
            os << std::string_view(sv.begin(), sv.end());
            if (os)
                return Result<void>::success();
            else
                return Result<void>::failure();
        }

        Result<void> putchar(char ch)
        {
            os << ch;
            if (os)
                return Result<void>::success();
            else
                return Result<void>::failure();
        }
    };
    static_assert(IsOstream<char, StdOstream>);
}

