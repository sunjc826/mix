#pragma once
#include <config.h>

#include <cstddef>
#include <cstdint>

// The actual configured size of a MIX byte
constexpr size_t byte_size = MIX_BYTE_SIZE;

#undef MIX_BYTE_SIZE
