#!/bin/bash

# Extremely simple configure script, mainly to support out of source build
# Doesn't do the extensive checks that automake configure scripts do.

SCRIPT_INVOCATION_DIR=$(pwd)
SCRIPT_DIR=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
NUM_THREADS=
CONFIG_CFLAGS=
CONFIG_CXX_FLAGS=
CONFIG_LDFLAGS=
while test $# -gt 0
do
    SHIFT_BY=1
    case "$1" in
    --threads)
        NUM_THREADS=$2
        : $((SHIFT_BY++))
        ;;
    --cflags)
        CONFIG_CFLAGS=$2
        : $((SHIFT_BY++))
        ;;
    --cxxflags)
        CONFIG_CXX_FLAGS=$2
        : $((SHIFT_BY++))
        ;;
    --ldflags)
        CONFIG_LDFLAGS=$2
        : $((SHIFT_BY++))
        ;;
    --)
        shift
        break
        ;;
    *)
        echo "Unrecognized option $1"
        ;;
    esac
    shift "$SHIFT_BY"
done

REMAINING_ARGS=("$@")

MAKE_ARGS=("${REMAINING_ARGS[@]}")

if test -n "$NUM_THREADS"
then
    MAKE_ARGS+=(-j"$NUM_THREADS")
fi

if test -n "$CONFIG_CFLAGS"
then
    MAKE_ARGS+=(CFLAGS="$CONFIG_CFLAGS")
fi

if test -n "$CONFIG_CXX_FLAGS"
then
    MAKE_ARGS+=(CXXFLAGS="$CONFIG_CXX_FLAGS")
fi

if test -n "$CONFIG_LDFLAGS"
then
    MAKE_ARGS+=(LDFLAGS="$CONFIG_LDFLAGS")
fi

set -x
echo "Configuring in $SCRIPT_INVOCATION_DIR"
sed -r \
    -e 's|@SRC_DIR@|'"$SCRIPT_DIR"'|' \
    -e 's|@MAKE_ARGS@|'"${MAKE_ARGS[*]}"'|' \
    "$SCRIPT_DIR"/template.mk > "$SCRIPT_INVOCATION_DIR"/Makefile

