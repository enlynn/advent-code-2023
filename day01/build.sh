#!/bin/sh

HOST_DIR=$(pwd)

COMMON="-Wall -Wno-unused-variable -Wno-missing-braces -Wno-unused-function -Wno-switch -Wno-unused-command-line-argument -Werror -Wvla -Wgnu-folding-constant"
DEBUG="-DDEBUG_BUILD -g"
RELEASE="-DOPTIMIZATION_BUILD -O3"

FLAGS=

MODE=
if [ "$1" = "release" ]; then
    MODE=release
    FLAGS="$COMMON $RELEASE"
else
    MODE=debug
    FLAGS="$COMMON $DEBUG"
fi

echo Building in $MODE mode.

echo clang $FLAGS code/main.c -o advent
clang $FLAGS code/main.c -o advent