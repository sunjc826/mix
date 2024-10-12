#!/bin/bash

# Extremely simple configure script, mainly to support out of source build
# Doesn't do the extensive checks that automake configure scripts do.
set -e
configure_main()
{
local script_invocation_dir
script_invocation_dir=$(pwd)
local script_dir
script_dir=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
pushd "$script_dir"
# shellcheck source-path=SCRIPTDIR
source lib.sh

push_scope
add_cleanup popd

local num_threads=
local config_cflags=
config_cxxflags=
config_ldflags=
while test $# -gt 0
do
    local shift_by
    shift_by=1
    case "$1" in
    --parallel)
        num_threads=$(nproc)
        ;;
    --threads)
        num_threads=$2
        : $((shift_by++))
        ;;
    --cflags)
        config_cflags=$2
        : $((shift_by++))
        ;;
    --cxxflags)
        config_cxxflags=$2
        : $((shift_by++))
        ;;
    --ldflags)
        config_ldflags=$2
        : $((shift_by++))
        ;;
    --)
        shift
        break
        ;;
    *)
        echo "Unrecognized option $1"
        ;;
    esac
    shift "$shift_by"
done

remaining_args=("$@")

make_args=("${remaining_args[@]}")

if test -n "$num_threads"
then
    make_args+=(-j"$num_threads")
fi

if test -n "$config_cflags"
then
    make_args+=(CFLAGS="$config_cflags")
fi

if test -n "$config_cxxflags"
then
    make_args+=(CXXFLAGS="$config_cxxflags")
fi

if test -n "$config_ldflags"
then
    make_args+=(LDFLAGS="$config_ldflags")
fi

set -x

echo "Configuring in $script_invocation_dir"

# https://stackoverflow.com/questions/4219255/how-do-you-get-the-list-of-targets-in-a-makefile
local make_targets
make_targets=($(make -qp | awk -F':' '/^[a-zA-Z0-9][^$#\/\t=]*:([^=]|$)/ {split($1,A,/ /);for(i in A)print A[i]}' | sort -u
))

sed -r \
    -e 's|@SRC_DIR@|'"$script_dir"'|' \
    -e 's|@MAKE_ARGS@|'"${make_args[*]}"'|' \
    -e 's|@MAKE_TARGETS@|'"${make_targets[*]}"'|' \
    "$script_dir"/template.mk > "$script_invocation_dir"/Makefile

if git rev-parse --is-inside-work-tree
then
    printf "*\n" > "$script_invocation_dir"/.gitignore
fi

set +x

pop_scope

return 0
}

configure_main "$@"
