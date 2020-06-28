#include "threadpool.h"

#ifdef __DEBUG
    #define ERROR_LOG(template,error_code) fprintf(stderr, "\x1B[31m"); \
                            fprintf(stderr, template, error_code); \
                            fprintf(stderr, "\x1B[0m")
#else
    #define ERROR_LOG(template,error_code) 
#endif

#ifdef __LOG
    #define LOG(template,log_data)  fprintf(stdout, "\x1B[34m"); \
                            fprintf(stdout, template, log_data); \
                            fprintf(stdout, "\x1B[0m")
    #define WARNING(template,log_data)  fprintf(stdout, "\x1B[33m"); \
                            fprintf(stdout, template, log_data); \
                            fprintf(stdout, "\x1B[0m")
#else
    #define LOG(template,log_data) 
    #define WARNING(template,log_data) 
#endif

typedef struct threadpool_info
{
    uint32_t tasks_complited;

}threadpool_info __attribute__ ((aligned (sizeof(void*))));

typedef struct thread
{
    pthread_t pthread;
    pthread_attr_t pthread_attr;
    uint64_t time_start;
}thread;

typedef struct thread_queue
{
    
};


volatile static threadpool_info __threadpool_info;
 
volatile static task *__task_queue_ = NULL;

volatile static task *__finished_task_queue = NULL;

static thread *thread_array;
static size_t thread_array_size;

volatile static pthread_mutex_t __mutex;
volatile static pthread_mutexattr_t __mutex_attr;
/*
Not interlocked. Better way change only in main thread.
*/
volatile static bool auto_clean = false;



/*
    Local function declaration
*/

void __attribute__((__cdecl__)) __threadpool_thread_handler(void);





void threadpool_init(const int number_of_threads)
{
    int threads_to_be_created = number_of_threads;
    if(number_of_threads > MAX_ALOWED_THREAD_NUMBER)
    {
        WARNING("[threadpool] Too many threads!\n", 0);
        threads_to_be_created = MAX_ALOWED_THREAD_NUMBER;
    }

    if(!interlocked_init())
    {
        ERROR_LOG("Interlocked init error!", 0);
        return;
    }

    pthread_mutex_init(&__mutex, &__mutex_attr);

    thread_array_size = threads_to_be_created;
    thread_array = malloc(sizeof(thread) * thread_array_size);

    memset(thread_array, 0, thread_array_size);

    for(int i = 0; i < threads_to_be_created; i++)
    {
        
        int thread_create_result = pthread_create(
                    &thread_array[i].pthread, 
                    &thread_array[i].pthread_attr, 
                    __threadpool_thread_handler, NULL);
        
        if(thread_create_result != 0)
        {
            ERROR_LOG("[threadpool] Error creating thread with code: %d\n", thread_create_result);
            return;
        }

        thread_array[i].time_start = clock();

        LOG("Thread %u created!\n", thread_array[i].pthread);
    }   
}

void threadpool_destroy()
{
    for(int i = 0; i < thread_array_size; i++)
        pthread_cancel(thread_array[i].pthread);

    free(thread_array);

    interlocked_free();

    pthread_mutex_destroy(&__mutex);
    thread_array_size = 0;
}

void threadpool_free_task(task *__task)
{
    if(!__task)
        return;
    if(__task->args)
        free(__task->args);
    if(__task->result)
        free(__task->result);
    free(__task);
}

void threadpool_init_task(task *__task)
{
    __task = malloc(sizeof(task));
    memset(__task, 0, sizeof(task));
}

task* threadpool_create_new_task(task_callback __func, task_args __args)
{
    task *task_pointer;
    threadpool_init_task(task_pointer);
    task_pointer->callback = __func;
    task_pointer->args = __args;
    return task_pointer;
}

task *__threadpool_dequeue_task()
{
    task *__tmp_task_pointer = NULL;
    while(__tmp_task_pointer == NULL)
    {
        while(pthread_mutex_trylock(&__mutex))
            for(volatile short i = 0; i < MAX_ALOWED_THREAD_NUMBER; i++);
        
        
        pthread_mutex_unlock(&__mutex);
    }
    return __tmp_task_pointer;
}

void threadpool_enqueue_task(task *__task)
{

}

task *threadpool_enqueue_finished_task(task *__task)
{

}

void __threadpool_enqueue_finished_task(task *__task)
{

}

void __threadpool_thread_handler(void)
{
    while(true)
    {
        task *cur_task = __threadpool_dequeue_task();
        cur_task->time_start = clock();
        
        task_result result = cur_task->callback(cur_task->args);

        cur_task->time_end = clock();
        
        cur_task->result = result;

        if(auto_clean)
            threadpool_free_task(cur_task);
        else
            __threadpool_enqueue_finished_task(cur_task);
    }
}