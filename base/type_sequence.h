#pragma once
#include <type_traits>
#include <tuple>
#include <cstddef>
namespace mix
{

    template <typename ...Ts>
    struct TypeSequence
    {
        static constexpr bool size = sizeof...(Ts);
    };

    template <size_t i, typename ...Ts>
    struct TypeSequenceGet
    {
        using type = std::tuple_element_t<i, std::tuple<Ts...>>();
    };

    template <typename T>
    struct TypeSequencePopFront;

    template <typename T, typename ...Ts>
    struct TypeSequencePopFront<TypeSequence<T, Ts...>>
    {
        using type = TypeSequence<Ts...>;
    };

    template <typename ListT, typename PushT>
    struct TypeSequencePushBack;

    template <typename T, typename ...Ts>
    struct TypeSequencePushBack<TypeSequence<Ts...>, T>
    {
        using type = TypeSequence<Ts..., T>;
    };

    template <typename PushT, typename ListT>
    struct TypeSequencePushFront;

    template <typename T, typename ...Ts>
    struct TypeSequencePushFront<T, TypeSequence<Ts...>>
    {
        using type = TypeSequence<T, Ts...>;
    };

    template <typename ListT>
    struct TypeSequencePopBack;

    template <typename T>
    struct TypeSequencePopBack<TypeSequence<T>>
    {
        using type = TypeSequence<>;
    };

    template <typename T, typename ...Ts>
    struct TypeSequencePopBack<TypeSequence<T, Ts...>>
    {
        using type = TypeSequencePushFront<T, typename TypeSequencePopBack<TypeSequence<Ts...>>::type>::type;
    };

    template <typename ListT>
    struct TypeSequenceFront;

    template <typename T, typename ...Ts>
    struct TypeSequenceFront<TypeSequence<T, Ts...>>
    {
        using type = T;
    };

    template <typename ListT>
    struct TypeSequenceBack;

    template <typename T, typename ...Ts>
    struct TypeSequenceBack<TypeSequence<T, Ts...>>
    {
        using type = std::conditional_t<sizeof...(Ts) == 0, std::type_identity<T>, TypeSequenceBack<TypeSequence<Ts...>>>::type;
    };
}
