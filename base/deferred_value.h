#pragma once
#include <array>
#include <memory>
#include <utility>
namespace mix
{

// Similar to optional but doesn't check for existence
// Assumes object exists when used.
// Assumes object is constructed exactly once.
template <typename T>
struct DeferredValue
{
    char buf[sizeof(T)];

    DeferredValue() = default;
    DeferredValue(DeferredValue<T> const &) = delete;
    DeferredValue(DeferredValue<T> &&) = delete;

    template <typename ...Args>
    void construct(Args &&...args)
    {
        std::construct_at(reinterpret_cast<T *>(buf), std::forward<Args>(args)...);
    }

    operator T const &() const
    {
        return *reinterpret_cast<T const *>(buf);
    }

    operator T &()
    {
        return *reinterpret_cast<T *>(buf);
    }

    operator T &&() &&
    {
        return std::move(*reinterpret_cast<T *>(buf));
    }

    ~DeferredValue()
    {
        std::destroy_at(reinterpret_cast<T *>(buf));
    }
};

namespace details
{
    template <typename ToT, typename FromT, size_t ...Is>
    [[gnu::always_inline]]
    std::array<ToT, sizeof...(Is)>
    convert_helper(std::array<FromT, sizeof...(Is)> const &arr, std::index_sequence<Is...>)
    {
        return { ToT(arr[Is])... };
    }
}

template <typename ToT, typename FromT, size_t size>
std::array<ToT, size>
convert(std::array<FromT, size> const &arr)
{
    return details::convert_helper(arr, std::make_index_sequence<size>());
}

// This is called unrolled because the generated machine code
// literally does `size` assignments in `convert_helper`. 
template <typename T, size_t size>
std::array<T, size>
actualize_unrolled(std::array<DeferredValue<T>, size> const &arr)
{
    return convert<T, DeferredValue<T>, size>(arr);
}

// This looks extremely unsafe. Probably undefined behavior in C++.
template <typename T, size_t size>
std::array<T, size> const &
actualize_reinterpret(std::array<DeferredValue<T>, size> const &arr)
{
    return reinterpret_cast<std::array<T, size> const &>(arr);
}

}
