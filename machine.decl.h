#pragma once
#include <base.h>

class Machine;
struct Op;

enum class OverflowPolicy
{
    set_overflow_bit,
    do_nothing,
    throw_exception,
};

enum CompareResult
{
    cr_less,
    cr_equal,
    cr_greater,
};
