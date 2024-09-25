#pragma once
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
        std::construct_at<T>(buf, std::forward<Args>(args)...);
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
        std::destroy_at<T>(buf);
    }
};

}
