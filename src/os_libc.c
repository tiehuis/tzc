#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/stat.h>   // open fdopen
#include <fcntl.h>      // O_CREAT O_RDWR

void _Noreturn std_exit(int code)
{
    exit(code);
}

__attribute__((format(printf, 1, 2)))
int std_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vprintf(fmt, args);
    va_end(args);
    return r;
}

__attribute__((format(printf, 2, 3)))
int std_fprintf(void *fd, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = vfprintf(fd, fmt, args);
    va_end(args);
    return r;
}

int std_vprintf(const char *fmt, va_list args)
{
    return vprintf(fmt, args);
}

__attribute__((format(printf, 1, 2)))
void _Noreturn std_panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(1);
}

void* std_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void* std_malloc(size_t size)
{
    return malloc(size);
}

char* std_readFile(const char *filename, long *fsize)
{
    FILE *fd = fopen(filename, "rb");
    if (!fd) return NULL;
    fseek(fd, 0, SEEK_END);
    *fsize = ftell(fd);
    rewind(fd);
    char *source = malloc(*fsize + 1);
    if (!source) return NULL;
    fread(source, *fsize, 1, fd);
    fclose(fd);
    source[*fsize] = 0;
    return source;
}

char* std_readFileRelative(const char *dir, const char *filename, long *fsize)
{
    char joined[512];
    snprintf(joined, sizeof(joined) - 1, "%s/%s", dir, filename);
    return std_readFile(joined, fsize);
}

void* std_createFile(const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) std_panic("failed to open %s", filename);
    void *fh = fdopen(fd, "wb");
    return fh;
}

size_t std_writeFile(void *ptr, size_t size, size_t nitems, void *fh)
{
    return fwrite(ptr, size, nitems, fh);
}