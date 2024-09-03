#pragma once
#include <memory>
#include <type_traits>
#include <utility>
#include <iostream>

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
        char value_[sizeof(Value)];
        char error_[sizeof(Error)];
    };

public:
    template <typename ...Ts>
    static Result success(Ts &&...value) 
    {
        Result result;
        result.is_success_ = true;
        std::construct_at(&result.value_, std::forward<Ts...>(value)...);
        return result;
    }

    template <typename ...Ts>
    static Result failure(Ts &&...error)
    {
        Result result;
        result.is_success_ = false;
        std::construct_at(&result.error_, std::forward<Ts...>(error)...);
        return result;
    }

    bool is_success() const
    {
        return is_success_;
    }

    operator bool() const
    {
        return is_success();
    }

    Value &value()
    {
        return *reinterpret_cast<Value *>(&value_);
    }

    Value const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    Error &error()
    {
        return *reinterpret_cast<Error *>(&error_);
    }

    Error const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }

    operator Value &()
    {
        return value();
    }

    operator Value const &() const
    {
        return value();
    }
};

template <typename Error>
class Result<void, Error>
{
    bool is_success_;
    char error_[sizeof(Error)];
public:
    static Result success()
    {
        Result result;
        result.is_success_ = true;
        return result;
    }
    
    template <typename ...Ts>
    static Result failure(Ts &&...error)
    {
        Result result;
        result.is_success_ = false;
        std::construct_at(&result.error_, std::forward<Ts...>(error)...);
        return result;
    }

    bool is_success() const
    {
        return is_success_;
    }

    operator bool() const
    {
        return is_success();
    }

    Error &error()
    {
        return *reinterpret_cast<Error *>(&error_);
    }

    Error const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }
};

template <typename Value>
class Result<Value, void>
{
    bool is_success_;
    char value_[sizeof(Value)];
public:
    template <typename ...Ts>
    static Result success(Ts &&...value)
    {
        Result result;
        result.is_success_ = true;
        std::construct_at(&result.value_, std::forward<Ts...>(value)...);
        return result;
    }

    static Result failure()
    {
        Result result;
        result.is_success_ = false;
        return result;
    }

    bool is_success() const
    {
        return is_success_;
    }

    operator bool() const
    {
        return is_success();
    }

    Value &value()
    {
        return reinterpret_cast<Value *>(value);
    }

    Value const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    operator Value &()
    {
        return value();
    }

    operator Value const &() const
    {
        return value();
    }
};

template <>
class Result<void, void>
{
    bool is_success_;
public:
    static Result success()
    {
        Result result;
        result.is_success_ = true;
        return result;
    }

    static Result failure()
    {
        Result result;
        result.is_success_ = false;
        return result;
    }

    bool is_success() const
    {
        return is_success_;
    }

    operator bool() const
    {
        return is_success();
    }
};

inline std::ostream &g_logger = std::cout;
