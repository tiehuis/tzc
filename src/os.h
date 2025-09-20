// wrapper over libc functions.
//
// The compiler should never directly call libc functions. It only accesses
// i/o via the following functions.

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void _Noreturn std_exit(int);
int std_printf(const char *fmt, ...);
int std_fprintf(void *fd, const char *fmt, ...);
int std_vprintf(const char *fmt, va_list args);
void _Noreturn std_panic(const char *fmt, ...);
void* std_realloc(void*, size_t);
void* std_malloc(size_t);
char* std_readFile(const char *filename, long *fsize);
void* std_createFile(const char *filename);
size_t std_writeFile(void *ptr, size_t size, size_t nitems, void *fh);