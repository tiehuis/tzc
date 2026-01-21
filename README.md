# tzc (tiny zig compiler)

A minimal compiler for a subset zig programming language.

End-goal is to compile the standard zig compiler as an alternative means
to the existing bootstrap process.

Bugs are expected. The only goal is to compile the zig compiler successfully,
any other bugs compiling other projects are considered out of scope.

## Status

 - [x] Tokenizer
 - [x] Parser
 - [ ] (10%) Ir
 - [ ] (0%) ComptimeEval
 - [ ] (10%) CodeGen

## Dependencies

 - c compiler (assumes zig cc is available)
 - sh

## Overview

This implements a complete self-contained zig compiler which targets C. Since it
is also written in C, the only required dependencies are a C compiler. libc is
not strictly required, assuming platform code is implemented by a corresponding
`os_*` file.

A minimal bash script is also provided which performs the full compilation to
output binary. See examples in the `test` directory: This assumes you have a
complete zig compiler with a lib directory accessible as this is not bundled.

```
./build.sh
./tzc.sh test/cg4.zig ../zig/lib
./a.out
Hello 0!
Hello 1!
Hello 2!
Hello 3!
Hello 4!
Hello 5!
Hello 6!
Hello 7!
Hello 8!
Hello 9!
```

---

The idealized goal is as below:

```
cc -o tzc src/tzc.c src/os.c                    # compile tzc
tzc zig/src/main.zig -o zig1.c -lib zig/lib     # compile zig compiler to c
cc -o zig1 zig1.c                               # compile generated c to machine code
./zig1 zig/src/main.zig -o zig2                 # compile stage2 using the original compiler
./zig2 zig/src/main.zig -o zig3                 # compile stage3 (with full optimizations)
```
