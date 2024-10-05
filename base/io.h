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
    };

    template <typename CharT, typename IstreamT>
    concept IsIstream = requires(IstreamT is)
    {
        {
            is.recv()
        } -> std::same_as<Result<std::span<CharT>>>;
    };

    template <typename FromT, typename ToT, typename TransformerT>
    concept IsTransformer = requires(TransformerT transformer)
    {
        {
            transformer.transform(std::declval<std::span<FromT>>())
        } -> std::same_as<Result<std::span<ToT>>>; 
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
            Result<std::span<ToT>> transform_result = transformer.transform(sv);
            if (!transform_result)
                return Result<void>::failure();
            downstream.send(transform_result.value());
            return Result<void>::success();
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

        Result<std::span<ToT>> recv()
        {
            using ResultType = Result<std::span<ToT>>;
            Result<std::span<FromT>> msg = upstream.recv();
            if (!msg)
                return ResultType::failure();
            return transformer.transform(msg.value());
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
    };
    static_assert(IsOstream<char, StdOstream>);
}

