#!/bin/sh

HOST_DIR=$(pwd)

COMMON="-Wall -Wno-unused-variable -Wno-missing-braces -Wno-unused-function -Wno-switch -Wno-unused-command-line-argument -Werror -Wvla -Wgnu-folding-constant"
DEBUG="-DDEBUG_BUILD -g"
RELEASE="-DOPTIMIZATION_BUILD -O3"

FLAGS=

MODE=
if [ "$1" = "release" ]; then
    MODE=release
    FLAGS="$COMMON $DEBUG"
else
    MODE=debug
    FLAGS="$COMMON $RELEASE"
fi

echo Building in $MODE mode.

clang $FLAGS code/main.c -o advent