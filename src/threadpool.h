



#ifndef __THREADPOOL_H__
#define __THREADPOOL_G__


#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_ALOWED_THREAD_NUMBER    100

#if MAX_ALOWED_THREAD_NUMBER <= 0
    #error Set max number of threads for pool
#endif

typedef void *task_args;
typedef void *task_result;
typedef void *(*task_callback)(task_args);

typedef struct __attribute__ ((__packed__)) task
{
    task_callback callback;
    task_args args;

    task_result result;
    uint64_t time_start;
    uint64_t time_end;
}task __attribute__((aligned));

typedef struct __queue_task
{
    task *cur_task;
    struct __queue_task *next_task;
}__queue_task;

typedef struct thread
{
    pthread_t pthread;
    pthread_attr_t pthread_attr;
    uint64_t time_start;
}thread;

typedef struct __task_queue
{
    __queue_task *front;
    __queue_task *back;
}__task_queue;

typedef struct __attribute__ ((__packed__)) threadpool_t
{
    thread **__threads;
    pthread_mutex_t __mutex;
    pthread_mutexattr_t __mutex_attr;
    volatile uint32_t threads_alive;
    volatile uint32_t threads_working;
    volatile uint32_t number_of_threads;
    volatile uint64_t tasks_done;
    volatile __task_queue *__task_queue;
}threadpool_t __attribute__ ((aligned (sizeof(void*))));


/*
@                   Function declaration
@   Used standart C notation with cdecl convention
*/

void __attribute__((cdecl)) 
        threadpool_init(threadpool_t *__restrict__ __threadpool, const int number_of_threads)
            __attribute__((force_align_arg_pointer)) __attribute__((__nonnull));

void __attribute__((cdecl)) 
        threadpool_destroy(threadpool_t *__restrict__ __threadpool)
            __attribute__((force_align_arg_pointer)) __attribute__((__nonnull));

void __attribute__((cdecl)) 
    threadpool_init_task(task *__restrict__ __task)
            __attribute__((force_align_arg_pointer)) __attribute__((__nonnull));

void __attribute__((cdecl)) 
        threadpool_enqueue_task(threadpool_t *__restrict__ __threadpool, 
                                task *__restrict__ __task)  
            __attribute__((force_align_arg_pointer)) __attribute__((__nonnull));

task* __attribute__((cdecl)) 
        threadpool_create_new_task(task_callback __func, task_args __args);

#endif