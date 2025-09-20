tzc: src/main.c src/os.c src/*.h
	zig cc -Os -g -std=c99 -Wall -Wextra -o tzc src/main.c src/os.c

clean:
	rm tzc

.PHONY: clean
