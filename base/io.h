#pragma once
#include <base/types.h>
#include <base/error.h>
#include <base/character_set.h>
#include <base/validated_int.h>
#include <limits>

namespace mix
{
    using istream = std::basic_istream<NativeByte>;
    using ostream = std::basic_ostream<NativeByte>;
};
