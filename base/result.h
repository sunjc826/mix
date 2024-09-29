#pragma once
#include <memory>
#include <utility>
namespace mix
{

template <typename Value, typename Error = void>
class Result
{
    static const auto identity_error_transform = [](Error error) {return error;};
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
        // Unfortunately, not friend of ValidatedInt/ValidatedWord
        std::construct_at(&result.value_, std::forward<Ts>(value)...);
        // Unfortunately, not constexpr
        // new (&result.value_) Value (std::forward<Ts>(value)...);
        // It seems that neither function fully meets our needs.
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

    template <typename ValueTransformT, typename ErrorTransformT>
    Result<decltype(ValueTransformT::operator()(std::declval<Value>())), decltype(ErrorTransformT::operator()(std::declval<Error>()))> 
    transform(ValueTransformT &&value_transform, ErrorTransformT &&error_transform = identity_error_transform)
    {
        using ResultType = Result<decltype(ValueTransformT::operator()(std::declval<Value>())), decltype(ErrorTransformT::operator()(std::declval<Error>()))>;
        if (is_success_)
            return ResultType::success(value_transform(value_));
        else
            return ResultType::failure(error_transform(error_));
    }

    template <typename ValueTransformT>
    Result<decltype(ValueTransformT::operator()(std::declval<Value>())), void> 
    transform_value(ValueTransformT &&value_transform)
    {
        using ResultType = Result<decltype(ValueTransformT::operator()(std::declval<Value>()))>;
        if (is_success_)
            return ResultType::success(value_transform(value_));
        else
            return ResultType::failure();
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
    static const auto identity_error_transform = [](Error error) {return error;};
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

    template <typename ValueTransformT, typename ErrorTransformT>
    Result<void, decltype(ErrorTransformT::operator()(std::declval<Error>()))> 
    transform(ErrorTransformT &&error_transform = identity_error_transform)
    {
        using ResultType = Result<void, decltype(ErrorTransformT::operator()(std::declval<Error>()))>;
        if (is_success_)
            return ResultType::success();
        else
            return ResultType::failure(error_transform(error_));
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
        // new (&result.value_) Value (std::forward<Ts>(value)...);
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

    template <typename ValueTransformT, typename ErrorT>
    Result<decltype(ValueTransformT::operator()(std::declval<Value>())), ErrorT> 
    transform(ValueTransformT &&value_transform, ErrorT error) const
    {
        using ResultType = Result<decltype(ValueTransformT::operator()(std::declval<Value>())), ErrorT>;
        if (is_success_)
            return ResultType::success(value_transform(value_));
        else
            return ResultType::failure(error);
    }

    template <typename ValueTransformT>
    Result<decltype(std::declval<ValueTransformT>()(std::declval<Value>()))> 
    transform_value(ValueTransformT &&value_transform) const
    {
        using ResultType = Result<decltype(ValueTransformT::operator()(std::declval<Value>()))>;
        if (is_success_)
            return ResultType::success(value_transform(value_));
        else
            return ResultType::failure();
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

template <typename T>
struct IsResult
{
    static constexpr bool value = false;
};

template <typename ValueT, typename ErrorT>
struct IsResult<Result<ValueT, ErrorT>>
{
    static constexpr bool value = true;
};

}
