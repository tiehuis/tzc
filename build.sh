#!/bin/sh

build() {
	zig cc -fsanitize=undefined -Os -g -std=c99 -Wall -Wextra -o tzc src/main.c src/os.c
}

case "$1" in
	"" | build) set -x;
		build
	;;

	test)
		build
		find test -type f -name '*.zig' | while IFS= read -r f; do
			./tzc "$f" -o "$f.c" -lib ../zig/lib
			b=$(basename "$f")
			n=$(dirname "$f")/${b%.zig}
			diff "$n.zig.c" "$n.e.c" || echo "$n: fail"
		done
	;;

	clean) set -x;
		rm -f tzc
		find test -type f -name '*.zig.c' -delete
	;;

	*)
		echo "usage: ./build.sh [build|clean]"
	;;
esac
