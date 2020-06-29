

#include <stdio.h>
#include <string.h>
#include "threadpool.h"

task_callback example_task(task_args args)
{
    for(volatile int i = 0; i < 10000000; i++);
}

int main(int argc, char **argv)
{   
    printf("Test started!\n");
    if(argc != 3)
    {
        fprintf(stderr, "\tIncorrect usage! Use:\n");
        fprintf(stderr, "\t<program_name> <threads> <tasks>\n");
        fprintf(stderr, "\tExample: /bin/test 50 1000\n");
        return -1;
    }
    threadpool_t first_pool;
    threadpool_init(&first_pool, atoi(argv[1]));
    int tasks = atoi(argv[2]);
    for(int i = 0; i <= tasks; i++)
        threadpool_enqueue_task(&first_pool, 
                        threadpool_create_new_task(example_task, NULL));

    while (1)
    {
        printf("\r%d", first_pool.tasks_done);
        if(first_pool.threads_working == 0)
            break;
    }
    printf("\n");
    threadpool_destroy(&first_pool);

    return 0;
}