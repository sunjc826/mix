#pragma once
namespace mix
{
// A list of application-level error codes
// All error codes are to be placed here instead of being scoped to a particular function/class/namespace
enum Error
{
    err_io,
    err_internal_logic,
    err_invalid_input,
    err_overflow,
    err_out_of_bounds,
    err_duplicate_symbol,
    err_missing_symbol,
};

}
