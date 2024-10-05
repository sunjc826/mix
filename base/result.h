#pragma once
#include <memory>
#include <utility>
namespace mix
{

template <typename T>
struct ResultTraits
{
    static constexpr bool is_result = false;
    using value_type = T;
};

template <typename ValueT, typename ErrorT>
struct Result;

template <typename ValueT, typename ErrorT>
struct ResultTraits<Result<ValueT, ErrorT>>
{
    static constexpr bool is_result = true;
    using value_type = ValueT;
};

template <typename ValueT, typename ErrorT = void>
class Result
{
    static const auto identity_error_transform = [](ErrorT error) {return error;};
    bool is_success_;
    union
    {
        ValueT value_;
        ErrorT error_;
    };

    constexpr
    Result() {}

public:
    using value_type = ValueT;
    using error_type = ErrorT;

    template <typename ...Ts>
    static constexpr
    Result success(Ts &&...value) 
    {
        Result result;
        result.is_success_ = true;
        // Unfortunately, not friend of ValidatedInt/ValidatedWord
        std::construct_at(&result.value_, std::forward<Ts>(value)...);
        // Unfortunately, not constexpr
        // new (&result.value_) ValueT (std::forward<Ts>(value)...);
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
    ValueT &value()
    {
        return value_;
    }

    constexpr
    ValueT const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    constexpr
    ErrorT &error()
    {
        return error_;
    }

    constexpr
    ErrorT const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }

    // Implicit conversions of this form are probably a really bad idea
    /* 
    constexpr
    operator ValueT &()
    {
        return value();
    }

    constexpr
    operator ValueT const &() const
    {
        return value();
    }
    */

    template <typename ValueTransformT, typename ErrorTransformT>
    Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type, decltype(ErrorTransformT::operator()(std::declval<ErrorT>()))> 
    transform(ValueTransformT &&value_transform, ErrorTransformT &&error_transform = identity_error_transform)
    {
        using ResultType = Result<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>())), decltype(ErrorTransformT::operator()(std::declval<ErrorT>()))>;
        if (is_success_)
        {
            if constexpr (ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::is_result)
                return value_transform(value_);
            else if constexpr (std::is_void_v<typename ResultType::value_type>)
                return ResultType::success();
            else
                return ResultType::success(value_transform(value_));
        }
        else
            return ResultType::failure(error_transform(error_));
    }

    template <typename ValueTransformT>
    Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type> 
    transform_value(ValueTransformT &&value_transform)
    {
        using ResultType = Result<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>;
        if (is_success_)
        {
            if constexpr (ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::is_result)
                return value_transform(value_);
            else if constexpr (std::is_void_v<typename ResultType::value_type>)
                return ResultType::success();
            else
                return ResultType::success(value_transform(value_));
        }
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

template <typename ErrorT>
class Result<void, ErrorT>
{
    static const auto identity_error_transform = [](ErrorT error) {return error;};
    bool is_success_;
    union
    {
        ErrorT error_;
    };

    constexpr
    Result() {}

public:
    using value_type = void;
    using error_type = ErrorT;

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
    ErrorT &error()
    {
        return error_;
    }

    constexpr
    ErrorT const &error() const
    {
        return REMOVE_CONST_FROM_PTR(this)->error();
    }

    template <typename ValueTransformT, typename ErrorTransformT>
    Result<void, decltype(ErrorTransformT::operator()(std::declval<ErrorT>()))> 
    transform(ErrorTransformT &&error_transform = identity_error_transform)
    {
        using ResultType = Result<void, decltype(ErrorTransformT::operator()(std::declval<ErrorT>()))>;
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

template <typename ValueT>
class Result<ValueT, void>
{
    bool is_success_;
    union
    {
        ValueT value_;
    };

    constexpr 
    Result() {}

public:
    using value_type = ValueT;
    using error_type = void;

    template <typename ...Ts>
    static constexpr
    Result success(Ts &&...value)
    {
        Result result;
        result.is_success_ = true;
        std::construct_at(&result.value_, std::forward<Ts>(value)...);
        // new (&result.value_) ValueT (std::forward<Ts>(value)...);
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
    ValueT &value()
    {
        return value_;
    }

    constexpr
    ValueT const &value() const
    {
        return REMOVE_CONST_FROM_PTR(this)->value();
    }

    // Implicit conversions of this form are probably a really bad idea
    /*
    constexpr
    operator ValueT &()
    {
        return value();
    }

    constexpr
    operator ValueT const &() const
    {
        return value();
    }
    */

    template <typename ValueTransformT, typename ErrorT>
    Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type, ErrorT> 
    transform(ValueTransformT &&value_transform, ErrorT error) const
    {
        using ResultType = Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type, ErrorT>;
        if (is_success_)
        {
            if constexpr (ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::is_result)
                return value_transform(value_);
            else if constexpr (std::is_void_v<typename ResultType::value_type>)
                return ResultType::success();
            else
                return ResultType::success(value_transform(value_));
        }
        else
            return ResultType::failure(error);
    }

    template <typename ValueTransformT>
    Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type> 
    transform_value(ValueTransformT &&value_transform) const
    {
        using ResultType = Result<typename ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::value_type>;
        if (is_success_)
        {
            if constexpr (ResultTraits<decltype(std::declval<ValueTransformT>()(std::declval<ValueT>()))>::is_result)
                return value_transform(value_);
            else if constexpr (std::is_void_v<typename ResultType::value_type>)
                return ResultType::success();
            else
                return ResultType::success(value_transform(value_));
        }
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
    using value_type = void;
    using error_type = void;

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



}
