#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#define REMOVE_CONST_FROM_PTR(ptr)\
    const_cast<std::remove_const_t<std::remove_reference_t<decltype(*ptr)>> *>(ptr)

template <bool is_view, typename T>
struct ConstIfView
{
    using type = T;
};

template <typename T>
struct ConstIfView<true, T>
{
    using type = T const;
};

template <bool is_view, typename T>
using ConstIfView_t = typename ConstIfView<is_view, T>::type;

template <typename Value, typename Error = void>
class Result
{
    bool is_success_;
    union
    {
        Value value_;
        Error error_;
    };

    constexpr
    Result() {}

public:
    template <typename ...Ts>
    static constexpr
    Result success(Ts &&...value) 
    {
        Result result;
        result.is_success_ = true;
        std::construct_at(&result.value_, std::forward<Ts...>(value)...);
        return result;
    }

    template <typename ...Ts>
    static constexpr
    Result failure(Ts &&...error)
    {
        Result result;
        result.is_success_ = false;
        std::construct_at(&result.error_, std::forward<Ts...>(error)...);
        return result;
    }

    constexpr 
    bool
    is_success() const
    {
        return is_success_;
    }

    constexpr
    operator bool() const
    {
        return is_success();
    }

    constexpr
    Value &value()
    {
        return value_;
    }

    constexpr
    Value const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    constexpr
    Error &error()
    {
        return error_;
    }

    constexpr
    Error const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }

    constexpr
    operator Value &()
    {
        return value();
    }

    constexpr
    operator Value const &() const
    {
        return value();
    }

    constexpr
    ~Result()
    {
        if (is_success())
            std::destroy_at(&value_);
        else
            std::destroy_at(&error_);
    }
};

template <typename Error>
class Result<void, Error>
{
    bool is_success_;
    union
    {
        Error error_;
    };

    constexpr
    Result() {}

public:
    static constexpr 
    Result success()
    {
        Result result;
        result.is_success_ = true;
        return result;
    }
    
    template <typename ...Ts>
    static constexpr
    Result failure(Ts &&...error)
    {
        Result result;
        result.is_success_ = false;
        std::construct_at(&result.error_, std::forward<Ts...>(error)...);
        return result;
    }

    constexpr
    bool 
    is_success() const
    {
        return is_success_;
    }

    constexpr
    operator bool() const
    {
        return is_success();
    }

    constexpr
    Error &error()
    {
        return error_;
    }

    constexpr
    Error const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }

    constexpr
    ~Result()
    {
        if (!is_success())
            std::destroy_at(&error_);
    }
};

template <typename Value>
class Result<Value, void>
{
    bool is_success_;
    union
    {
        Value value_;
    };

    constexpr 
    Result() {}

public:
    template <typename ...Ts>
    static constexpr
    Result success(Ts &&...value)
    {
        Result result;
        result.is_success_ = true;
        std::construct_at(&result.value_, std::forward<Ts...>(value)...);
        return result;
    }

    static constexpr 
    Result failure()
    {
        Result result;
        result.is_success_ = false;
        return result;
    }

    constexpr
    bool is_success() const
    {
        return is_success_;
    }

    constexpr
    operator bool() const
    {
        return is_success();
    }

    constexpr
    Value &value()
    {
        return value_;
    }

    constexpr
    Value const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    constexpr
    operator Value &()
    {
        return value();
    }

    constexpr
    operator Value const &() const
    {
        return value();
    }

    constexpr
    ~Result()
    {
        if (is_success())
            std::destroy_at(&value_);
    }
};

template <>
class Result<void, void>
{
    bool is_success_;
public:
    static constexpr 
    Result success()
    {
        Result result;
        result.is_success_ = true;
        return result;
    }

    static constexpr 
    Result failure()
    {
        Result result;
        result.is_success_ = false;
        return result;
    }

    constexpr
    bool 
    is_success() const
    {
        return is_success_;
    }

    constexpr
    operator bool() const
    {
        return is_success();
    }
};

inline std::ostream &g_logger = std::cout;

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
        std::construct_at<T>(buf, std::forward<Args...>(args)...);
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

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};
