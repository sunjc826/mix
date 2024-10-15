#pragma once
#include <base/result.h>
#include <optional>
#include <type_traits>
namespace mix
{
    template <typename T>
    class Optional : protected Result<T, void>
    {
    public:
        [[gnu::flatten]]
        constexpr
        Optional(std::nullopt_t)
            : Result<T>(std::false_type{})
        {}

        template <typename ...ArgTs>
        [[gnu::flatten]]
        constexpr
        Optional(ArgTs &&...args)
            : Result<T>(std::true_type{}, std::forward<ArgTs>(args)...)
        {}

        void transform()
        {
            
        }
    };

}