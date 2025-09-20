# tzc (tiny zig compiler)

A minimal compiler for a subset zig programming language.

End-goal is to compile the standard zig compiler as an alternative means
to the existing bootstrap process.

Bugs are expected. The only goal is to compile the zig compiler successfully,
any other bugs compiling other projects are considered out of scope.

## Status

 - [x] Tokenizer
 - [ ] Parser (~75%)
 - [ ] CodeGen (1%)

## Overview

This implements a complete self-contained zig compiler which targets C. Since it
is  also written in C, the only required dependencies are a C compiler. libc is
not strictly required, assuming platform code is implemented by a corresponding
`os_*` file.

A minimal bash script is also provided which performs the full compilation to
output binary.

The idealized goal is as below:

```
cc -o tzc src/tzc.c src/os.c                    # compile tzc
tzc zig/src/main.zig -o zig1.c -lib zig/lib     # compile zig compiler to c
cc -o zig1 zig1.c                               # compile generated c to machine code
./zig1 zig/src/main.zig -o zig2                 # compile stage2 using the original compiler
./zig2 zig/src/main.zig -o zig3                 # compile stage3 (with full optimizations)
```

## Implementation

```
Tokenizer.h -> Parser.h -> CodeGen.h
```

Semantic analysis is part of codegen for simplicity. The tokenizer is very
similar to the zig implementation. The parser and beyond differ a bit and don't
(currently) use data-oriented programming principles for implementation
simplicity. The primary goal is that this is easy to keep up-to-date and
readable, not overtly for performance.

## Stretch Goals

 - Memory optimization
 - Support historical zig versions
 - Direct backends (non-c)
