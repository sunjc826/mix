#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/validated_int.h>
#include <limits>

namespace mix
{

    template <typename CharT, typename OstreamT>
    concept is_ostream = requires(OstreamT os)
    {
        {
            os.send(std::declval<std::basic_string_view<CharT>>())
        } -> std::same_as<Result<void>>;
    };

    template <typename CharT, typename IstreamT>
    concept is_istream = requires(IstreamT is)
    {
        {
            is.recv()
        } -> std::same_as<Result<std::basic_string_view<CharT>>>;
    };

    template <typename FromT, typename ToT, typename TransformerT>
    concept is_transformer = requires(TransformerT transformer)
    {
        {
            transformer.transform(std::declval<std::basic_string_view<FromT>>())
        } -> std::same_as<Result<std::basic_string_view<ToT>>>; 
    };

    template <typename FromT, typename ToT, typename DownstreamT, typename TransformerT>
    requires is_ostream<ToT, DownstreamT> && is_transformer<FromT, ToT, TransformerT>
    class PushPipe
    {
        TransformerT transformer;
        DownstreamT &downstream;
    public:
        explicit PushPipe(DownstreamT &downstream)
            : downstream(downstream)
        {}

        Result<void> send(std::basic_string_view<FromT> sv)
        {
            Result<std::basic_string_view<ToT>> transform_result = transformer.transform(sv);
            if (!transform_result)
                return Result<void>::failure();
            downstream.send(transform_result.value());
            return Result<void>::success();
        }
    };

    template <typename FromT, typename ToT, typename UpstreamT, typename TransformerT>
    requires is_istream<FromT, UpstreamT> && is_transformer<FromT, ToT, TransformerT>
    class PullPipe
    {
        
        TransformerT transformer;
        UpstreamT &upstream;
    public:
        explicit PullPipe(UpstreamT &upstream)
            : upstream(upstream)
        {}

        Result<std::basic_string_view<ToT>> recv()
        {
            using ResultType = Result<std::basic_string_view<ToT>>;
            Result<std::basic_string_view<FromT>> msg = upstream.recv();
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

        Result<std::string_view> recv()
        {
            is >> s;            
            return Result<std::string_view>::success(s);
        }
    };
    static_assert(is_istream<char, StdIstream>);

    class StdOstream
    {
        std::ostream &os;
    public:
        StdOstream(std::ostream &os)
            : os(os)
        {}

        Result<void> send(std::string_view sv)
        {
            os << sv;
            if (os)
                return Result<void>::success();
            else
                return Result<void>::failure();
        }
    };
    static_assert(is_ostream<char, StdOstream>);

}
