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

template <typename ArgT, Validator<ArgT> fn>
using ValidatorToFunctor = FuncToFunctor<bool, ArgT, fn>;

template <IntValidator fn>
using IntValidatorToFunctor = ValidatorToFunctor<NativeInt, fn>;

}
