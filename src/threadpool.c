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


/*
Not interlocked. Better way change only in main thread.
*/
volatile static bool auto_clean = true;



/*
    Local function declaration
*/

void __attribute__((__cdecl__)) 
    __threadpool_thread_handler(void *__threadpool);
task* __attribute__((__cdecl__)) 
    __threadpool_dequeue_task(threadpool_t *__restrict__ __threadpool);

void __attribute__((__cdecl__)) 
    __threadpool_queue_push_n(threadpool_t *__threadpool, task *__task);


void threadpool_init(threadpool_t *__threadpool, const int number_of_threads)
{
    int threads_to_be_created = number_of_threads;
    if(number_of_threads > MAX_ALOWED_THREAD_NUMBER)
    {
        WARNING("[threadpool] Too many threads!\n", 0);
        threads_to_be_created = MAX_ALOWED_THREAD_NUMBER;
    }

    memset(__threadpool, 0, sizeof(threadpool_t));

    __threadpool->number_of_threads = threads_to_be_created;
    __threadpool->__threads = (thread**)
                    malloc(sizeof(thread*) * __threadpool->number_of_threads);

    __threadpool->__task_queue = (__task_queue*)malloc(sizeof(__task_queue));

    pthread_mutex_init(&__threadpool->__mutex, &__threadpool->__mutex_attr);

    memset(__threadpool->__threads, 0, __threadpool->number_of_threads);
    memset(__threadpool->__task_queue, 0, sizeof(__task_queue));

    for(int i = 0; i < __threadpool->number_of_threads; i++)
    {
        __threadpool->__threads[i] = (thread*)malloc(sizeof(thread));
        int thread_create_result = pthread_create(
                    &__threadpool->__threads[i]->pthread, 
                    &__threadpool->__threads[i]->pthread_attr, 
                    __threadpool_thread_handler, __threadpool);
        
        if(thread_create_result != 0)
        {
            ERROR_LOG("[threadpool] Error creating thread with code: %d\n", thread_create_result);
            return;
        }
        __threadpool->threads_alive++;
        pthread_detach(__threadpool->__threads[i]);

        __threadpool->__threads[i]->time_start = clock();
        LOG("Thread %u created!\n", __threadpool->__threads[i]->pthread);
    }   
    
}

void threadpool_destroy(threadpool_t *__threadpool)
{
    pthread_mutex_lock(&__threadpool->__mutex);
    for(int i = 0; i < __threadpool->number_of_threads; i++)
    {
        int cancel_res = pthread_cancel(__threadpool->__threads[i]->pthread);
        if(cancel_res != 0)
        {
            ERROR_LOG("Canceled thread with code: %d\n", cancel_res);
        }
        else
        {
            LOG("Thread %u has been canceled!\n", (__threadpool->__threads[i]->pthread));
        }
        
    }
    pthread_mutex_unlock(&__threadpool->__mutex);

    free(__threadpool->__threads);
    pthread_mutex_destroy(&__threadpool->__mutex);

    __queue_task *front_queue_task = __threadpool->__task_queue->front;

    while(front_queue_task)
    {
        __queue_task *buf_task = front_queue_task->next_task;
        threadpool_free_task(&front_queue_task->cur_task);
        free(front_queue_task);
        front_queue_task = buf_task;
    }

    LOG("Threadpool has been cleaned!\n", 0);
}

void threadpool_free_task(task *__task)
{
    if(!__task)
        return;
    free(__task);
    return;
}

void threadpool_init_task(task *__task)
{
    if(__task)
        memset(__task, 0, sizeof(task));
}

task* threadpool_create_new_task(task_callback __func, task_args __args)
{
    task *task_pointer = (task*)malloc(sizeof(task));
    threadpool_init_task(task_pointer);
    task_pointer->callback = __func;
    task_pointer->args = __args;
    return task_pointer;
}

void threadpool_enqueue_task(threadpool_t *__threadpool, task *__task)
{
    pthread_mutex_lock(&__threadpool->__mutex);
    __threadpool_queue_push_n(__threadpool, __task);
    pthread_mutex_unlock(&__threadpool->__mutex);
}


void __threadpool_queue_push_n(threadpool_t *__threadpool, task *__task)
{
    __queue_task *new_queue_task = (__queue_task*)malloc(sizeof(__queue_task));

    new_queue_task->cur_task = __task;
    if(!__threadpool->__task_queue->back)
    {
        __threadpool->__task_queue->back = new_queue_task;
        __threadpool->__task_queue->front = new_queue_task;
    }
    else
    {
        __threadpool->__task_queue->back->next_task = new_queue_task;
        __threadpool->__task_queue->back = new_queue_task;
    }
}

void __threadpool_thread_handler(void *__threadpool)
{
    threadpool_t *threadpool = (threadpool_t*)__threadpool;
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while(true)
    {
        task *cur_task = __threadpool_dequeue_task(threadpool);
        
        cur_task->time_start = clock();
        __sync_add_and_fetch(&threadpool->threads_working, 1);

        task_result result = cur_task->callback(cur_task->args);

        
        cur_task->time_end = clock();
        
        __sync_add_and_fetch(&threadpool->tasks_done, 1);
        __sync_add_and_fetch(&threadpool->threads_working, -1);

        cur_task->result = result;
        if(auto_clean)
            threadpool_free_task(cur_task);
    }
}

task *__threadpool_dequeue_task(threadpool_t *__threadpool)
{
    
    task *__tmp_task_pointer = NULL;
    while(__tmp_task_pointer == NULL)
    {
        while(pthread_mutex_trylock(&__threadpool->__mutex))
            for(volatile short i = 0; i < MAX_ALOWED_THREAD_NUMBER; i++);
        if(__threadpool->__task_queue->front){
            __tmp_task_pointer = __threadpool->__task_queue->front->cur_task;
            if(__threadpool->__task_queue->front == __threadpool->__task_queue->back)
                __threadpool->__task_queue->front = __threadpool->__task_queue->back = NULL;    
            else
                __threadpool->__task_queue->front = __threadpool->__task_queue->front->next_task;
        }
        pthread_mutex_unlock(&__threadpool->__mutex);
    }
    
    return __tmp_task_pointer;
    
}