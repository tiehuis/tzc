test {
    const w, const x, const y = .{ 0, 0, 0 };
    asm volatile (
        \\mov.4s v0, %[x]
        \\sha256h.4s %[x], %[y], %[w]
        \\sha256h2.4s %[y], v0, %[w]
        : [x] "=w" (x),
          [y] "=w" (y),
        : [_] "0" (x),
          [_] "1" (y),
          [w] "w" (w),
        : .{ .v0 = true });
}
