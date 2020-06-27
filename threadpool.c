#include "threadpool.h"

typedef struct threadpool_info
{
    uint32_t tasks_complited;

}threadpool_info __attribute__ ((aligned (sizeof(void*))));


volatile static threadpool_info __threadpool_info;
 
volatile static task *__task_queue = NULL;


void threadpool_init(const int number_of_threads)
{

}

void threadpool_free_task(task *)
{

}

void threadpool_init_task(task *)
{

}

task* threadpool_create_new_task()
{
    
}

task *__threadpool_dequeue_task()
{

}

void threadpool_enqueue_task(task *)
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
            
    }
}