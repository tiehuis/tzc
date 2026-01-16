#include "os_libc.c"

__attribute__((format(printf, 1, 2)))
int std_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = std_vprintf(fmt, args);
    va_end(args);
    return r;
}

__attribute__((format(printf, 2, 3)))
int std_fprintf(void *fd, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = std_vfprintf(fd, fmt, args);
    va_end(args);
    return r;
}

int std_vprintf(const char *fmt, va_list args)
{
    return std_vfprintf(stdout, fmt, args);
}

__attribute__((format(printf, 1, 2)))
void _Noreturn std_panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std_vprintf(fmt, args);
    va_end(args);
    assume(false);  // backtrace on panic with ubsan
    std_exit(1);
}

void* std_memcpy(void *src, const void *dst, size_t len)
{
    char *src_b = src;
    const char *dst_b = dst;
    for (size_t i = 0; i < len; i++) *src_b++ = *dst_b++;
    return src;
}

size_t std_strlen(const char *s)
{
    size_t i = 0;
    while (*s++) i++;
    return i;
}