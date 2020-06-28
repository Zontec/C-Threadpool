#include "interlocked.h"
#include <time.h>

#define MAX_ALOWED_THREAD_NUMBER    10

#if MAX_ALOWED_THREAD_NUMBER <= 0
    #error Set max number of threads for pool
#endif

typedef void *task_args;
typedef void *task_result;
typedef void *(*task_callback)(task_args);

typedef struct task
{
    task_callback callback;
    task_args args;

    task_result result;
    uint64_t time_start;
    uint64_t time_end;
}task;

void __attribute__((__cdecl__)) threadpool_init(const int number_of_threads);