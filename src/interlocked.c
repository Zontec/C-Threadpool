#include "interlocked.h"


#ifdef __DEBUG
    #define ERROR_LOG(template,error_code) fprintf(stderr, template, error_code)
#else
    #define ERROR_LOG(template,error_code)
#endif

#ifdef __LSB
    #define FOR(iter,size) for(iter = size - 1; iter >= 0; iter--)
#else
    #define FOR(iter,size) for(iter = 0; iter < size; iter++)
#endif

pthread_mutex_t __mutex;
pthread_mutexattr_t __mutex_attr;


bool interlocked_init(void)
{
    int mutex_create_result = pthread_mutex_init(&__mutex, &__mutex_attr);
    if(mutex_create_result != 0)
    {
        ERROR_LOG("[Interlocked] Mutex init error with code: %d\n", mutex_create_result);
        return false;
    }
    return true;
}

bool interlocked_free(void)
{
    int mutex_destroy_result = pthread_mutex_destroy(&__mutex);
    if(mutex_destroy_result != 0)
    {
        ERROR_LOG("[Interlocked] Mutex destroy error with code: %d\n", mutex_destroy_result);
        return false;
    }
    return true;  
}

void interlocked_set_var(volatile void *__dst, 
                    const volatile void  *__src, 
                    const size_t __size)
{
    pthread_mutex_lock(&__mutex);
    memcpy(__dst, __src, __size);
    pthread_mutex_unlock(&__mutex);
}


/*
__first > __second -> 1
__first < __second -> -1
__first == __second -> 0
*/
int interlocked_cmp(const volatile void *__first, 
                    const volatile void  *__second, 
                    const size_t __size)
{
    int res = 0;
    int i = 0;
    pthread_mutex_lock(&__mutex);
    FOR(i, __size)
    {
        unsigned char first_byte = *(volatile unsigned char *)__first;
        unsigned char second_byte = *(volatile unsigned char *)__second;
        if(first_byte > second_byte)
        {
            res = 1;
            break;
        }
        if(first_byte < second_byte)
        {
            res = -1;
            break;
        }
    }
    pthread_mutex_unlock(&__mutex);
    return res;
}

void *interlocked_function_exec(void *(*func)(void*), void *args)
{
    pthread_mutex_lock(&__mutex);
    void *func_res = func(args);
    pthread_mutex_unlock(&__mutex);
    return func_res;
}
                    