#pragma once
#include <memory>

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
        std::construct_at(&result.value_, std::forward<Ts>(value)...);
        return result;
    }

    template <typename ...Ts>
    static constexpr
    Result failure(Ts &&...error)
    {
        Result result;
        result.is_success_ = false;
        std::construct_at(&result.error_, std::forward<Ts>(error)...);
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

    // Implicit conversions of this form are probably a really bad idea
    /* 
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
    */

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
        std::construct_at(&result.error_, std::forward<Ts>(error)...);
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
        std::construct_at(&result.value_, std::forward<Ts>(value)...);
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

    // Implicit conversions of this form are probably a really bad idea
    /*
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
    */

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
