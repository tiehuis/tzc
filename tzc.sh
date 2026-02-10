#!/bin/sh

set -eu

[ "$#" -eq 2 ] || { echo "usage: $0 <input> <zig_lib_dir>" >&2; exit 1; }

./tzc "$1" -o "$1.c" -lib "$2"
zig cc -fsanitize=undefined -Os "$1.c"
