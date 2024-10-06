#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/result.h>

#include <limits>
#include <concepts>

namespace mix
{

    template <typename CharT, typename OstreamT>
    concept IsOstream = requires(OstreamT os)
    {
        {
            os.send(std::declval<std::span<CharT>>())
        } -> std::same_as<Result<void>>;

        {
            os.send_char(std::declval<CharT>())
        } -> std::same_as<Result<void>>;
    };

    template <typename CharT, typename IstreamT>
    concept IsIstream = requires(IstreamT is)
    {
        {
            is.recv()
        } -> std::same_as<Result<std::span<CharT>>>;

        {
            is.recv_char()
        } -> std::same_as<Result<std::optional<CharT>>>;
    };

    template <typename FromT, typename ToT, typename TransformerT>
    concept IsTransformer = requires(TransformerT transformer)
    {
        {
            transformer.send(std::declval<std::span<FromT>>())
        } -> std::same_as<Result<void>>;
        {
            transformer.send_char(std::declval<FromT>())
        } -> std::same_as<Result<void>>;
        {
            transformer.recv()
        } -> std::same_as<Result<std::span<ToT>>>;
        {
            transformer.recv_char()
        } -> std::same_as<Result<std::optional<ToT>>>;
    };

    template <typename FromT, typename ToT, typename DownstreamT, typename TransformerT>
    requires IsOstream<ToT, DownstreamT> && IsTransformer<FromT, ToT, TransformerT>
    class PushPipe
    {
        TransformerT transformer;
        DownstreamT &downstream;
    public:
        explicit PushPipe(DownstreamT &downstream)
            : downstream(downstream)
        {}

        Result<void> send(std::span<FromT> sv)
        {
            transformer.send(sv).transform_value([this]{
                return transformer.recv();
            }).transform_value([this](std::span<ToT> transformed_sv){
                return downstream.send(transformed_sv);
            });
        }

        Result<void> send_char(FromT ch)
        {
            return transformer.send_char(ch).transform_value([this]{
                return transformer.recv_char();
            }).transform_value([this](ToT ch){
                return downstream.send_char(ch);
            });
        }
    };

    template <typename FromT, typename ToT, typename UpstreamT, typename TransformerT>
    requires IsIstream<FromT, UpstreamT> && IsTransformer<FromT, ToT, TransformerT>
    class PullPipe
    {
        TransformerT transformer;
        UpstreamT &upstream;
    public:
        explicit PullPipe(UpstreamT &upstream)
            : upstream(upstream)
        {}

        [[gnu::flatten]]
        Result<std::span<ToT>> recv()
        {
            return upstream.recv().transform_value([this](std::span<FromT> sp){
                return transformer.send(sp);
            }).transform_value([this]{
                return transformer.recv();
            });
        }

        [[gnu::flatten]]
        Result<std::optional<ToT>> recv_char()
        {
            return upstream.recv().transform_value([this](std::span<FromT> sp){
                return transformer.send(sp);
            }).transform_value([this]{
                return transformer.recv_char();
            });
        }
    };

    class StdIstream
    {
        std::istream &is;
        std::string s;
    public:
        StdIstream(std::istream &is)
            : is(is)
        {}

        Result<std::span<char>> recv()
        {
            is >> s;            
            return Result<std::span<char>>::success(s);
        }

        Result<std::optional<char>> recv_char()
        {
            
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

        Result<void> send(std::span<char> sv)
        {
            os << std::string_view(sv.begin(), sv.end());
            if (os)
                return Result<void>::success();
            else
                return Result<void>::failure();
        }

        Result<void> send_char(char ch)
        {

        }
    };
    static_assert(IsOstream<char, StdOstream>);
}

