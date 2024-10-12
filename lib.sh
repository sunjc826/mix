#!/bin/bash

function __init_lib()
{
    __SCOPE_STACK=()
}

function push_scope()
{
    local scope_name=${1:-scope_${#__SCOPE_STACK[@]}}
    local stack_scope_name
    for stack_scope_name in "${__SCOPE_STACK[@]}"
    do
        if test "$stack_scope_name" = "$scope_name"
        then
            echo "Error: duplicate scope name" >&2
            return 1
        fi
    done
    __SCOPE_STACK+=( "$scope_name" )
    declare -a "__SCOPE_${scope_name}_CLEANUPS=()"
    echo "Pushed scope $scope_name"
}

function add_cleanup()
{
    if test "${#__SCOPE_STACK[@]}" -eq 0
    then
        echo "Error: Scope stack is currently empty" >&2
        return 1
    fi
    local last_scope_name=${__SCOPE_STACK[-1]}
    eval __SCOPE_"${last_scope_name}"_CLEANUPS+="('""$*""')"
    echo "Registered cleanup func $* to scope ${last_scope_name}"
}

function pop_scope()
{
    if test "${#__SCOPE_STACK[@]}" -eq 0
    then
        echo "Error: Scope stack is currently empty" >&2
        return 1
    fi
    local last_scope_name=${__SCOPE_STACK[-1]}
    local cleanup_funcs_name=__SCOPE_"${last_scope_name}"_CLEANUPS
    local cleanup_funcs
    eval cleanup_funcs=\( \${"$cleanup_funcs_name"[@]} \)
    local cleanup_func
    local idx
    # Run in reverse
    for ((idx=${#cleanup_funcs[@]}-1;idx>=0;idx--))
    do
        cleanup_func=${cleanup_funcs[idx]}
        echo "Running cleanup func: $cleanup_func"
        $cleanup_func
    done
    unset __SCOPE_"${last_scope_name}"_CLEANUPS
    unset "SCOPE_STACK[-1]"
}

if ! test -v "$__LIB_HAS_BEEN_SOURCED"
then
    __init_lib
    __LIB_HAS_BEEN_SOURCED=true
fi

