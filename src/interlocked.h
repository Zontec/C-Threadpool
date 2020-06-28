#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>

bool __attribute__((__cdecl__)) interlocked_init(void);

bool __attribute__((__cdecl__)) interlocked_free(void);

void __attribute__((__cdecl__)) interlocked_set_var(volatile void *__dst, 
                    const volatile void  *__src, 
                    const size_t __src_size) __attribute__((nonnull (1, 2)));

int __attribute__((__cdecl__)) interlocked_cmp(const volatile void *__first, 
                    const volatile void  *__second, 
                    const size_t __size) __attribute__((nonnull (1, 2)));