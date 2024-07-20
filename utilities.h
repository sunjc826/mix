#include <type_traits>
#define REMOVE_CONST_FROM_PTR(ptr)\
    const_cast<std::remove_const_t<std::remove_reference_t<decltype(*ptr)>> *>(ptr)
