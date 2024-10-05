#pragma once
#include <base/io.h>
#include <base/validation/v2.h>
namespace mix
{

// UTF-8 is a prefix code, and so we do not need lookahead.
// https://en.wikipedia.org/wiki/Self-synchronizing_code
struct MixIstream
{
    Result<std::span<ValidatedChar>> recv()
    {
        
    }

    Result<std::optional<ValidatedChar>> recv_char()
    {

    }
};

static_assert(IsIstream<ValidatedChar, MixIstream>);

struct MixOstream
{
    Result<void> send(std::span<ValidatedChar> sp)
    {

    }

    Result<void> send_char(ValidatedChar ch)
    {

    }
};

static_assert(IsOstream<ValidatedChar, MixOstream>);
}
