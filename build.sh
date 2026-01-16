#!/bin/sh

case "$1" in
	"" | build) set -x;
		zig cc -fsanitize=undefined -Os -g -std=c99 -Wall -Wextra -o tzc src/main.c src/os.c
	;;

	clean) set -x;
		rm -f tzc
	;;

	*)
		echo "usage: ./build.sh [build|clean]"
	;;
esac