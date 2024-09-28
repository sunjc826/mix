#pragma once
#include <base/types.h>

namespace mix
{

template <typename ReturnT, typename ArgT, ReturnT (*fn)(ArgT)>
struct FuncToFunctor
{
    [[gnu::flatten]]
    constexpr ReturnT operator()(ArgT i) const
    {
        return fn(i);
    }
};

template <typename ArgT, bool (*fn)(ArgT)>
using BoolFuncToFunctor = FuncToFunctor<bool, ArgT, fn>;

template <Validator fn>
using ValidatorToFunctor = BoolFuncToFunctor<NativeInt, fn>;

}
