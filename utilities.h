#include <type_traits>
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