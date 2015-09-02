/* 1) First:
 * Threads will wait for each other at a barrier point before proceed further.
 * The number of hits per barrier is specified during initialization. At the
 * end barriers' resources should be freed.
 *
 * 2) Second: globals per thread
 * pthread_key_create():
 * A key is visible to all threads but for each thread it returns different
 * value (index to TLS - thread's local storage). Thanks to that it is possible
 * to have dynamically allocated thread-specific data even without exact
 * ThreadData.id - key identifies thread unambiguously.
 *
 * 3)
 * pthread_once():
 * Sets onceFlag, initializes thread's key. Called only once from first thread
 * scheduled. This is solution instead of having mutexed flag - mutex
 * initialization is not always possible
 *
 * pthread_setspecific() / pthread_getspecific():
 * Returns value bound to key in that thread. Thread-specific data does not
 * have to be passed by pointers in ThreadData structure now. For different
 * threads the same key will return different data.
 *
 * threadDataDestructor gets previously assigned key value as an argument so it
 * is able to free the allocated memory. The key itself is already null inside
 * destructor function.
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREADS_NB 2
#define DATA_SZ 4096
#define CHECKER( invok, cond, label)do{                         \
        if(invok cond){ perror(#label); exit(EXIT_FAILURE); }   \
    }while(0)

typedef struct ThreadData
{
    char c;
    int id;
} ThreadData;

static pthread_barrier_t startBarrier, processBarrier;
static pthread_key_t key;
static void* threadMain(void *p);
static void keyInit(void);
static void threadDataDestructor(void* opaque);
static void setThreadData(ThreadData *td);
static void threadDataAllocator(void);

int main()
{
    pthread_t tid[THREADS_NB];
    ThreadData td[THREADS_NB] = { {'a',0}, {'b',1}};

    for(size_t i=0; i<THREADS_NB;i++)
        CHECKER(
            pthread_create((tid+i), NULL, threadMain, td+i),
            !=0, pthread_create );
    for(size_t i=0; i<2;i++)
        pthread_join(*(tid+i),NULL);
    return 0;
}

void __attribute__((constructor)) pthreadInits(void)
{
    CHECKER(
        pthread_barrier_init(&startBarrier, NULL, THREADS_NB),
        !=0, barrier_init );
    CHECKER(
        pthread_barrier_init(&processBarrier, NULL, THREADS_NB),
        !=0, barrier_init );
}

void __attribute__((destructor)) pthreadDestroys(void)
{
    pthread_barrier_destroy(&startBarrier);
    pthread_barrier_destroy(&processBarrier);
}

void* threadMain(void *opaque)
{
    static pthread_once_t onceFlag=PTHREAD_ONCE_INIT;
    pthread_once(&onceFlag, keyInit);
    // key is created - let's bind the data to it
    threadDataAllocator();
    pthread_barrier_wait(&startBarrier);
    // setting data to some value
    setThreadData(opaque);
    pthread_barrier_wait(&processBarrier);
    puts("-- thread-specific data has been allocated --");
    //accesing the data
    printf("TEST: %s\n", (char *)pthread_getspecific(key) );
    fflush(stdout);
    return NULL;
}

void keyInit(void)
{
    /* this is called only once per process and needs to initialize keys */
    CHECKER(
        pthread_key_create(&key, threadDataDestructor),
        !=0, key_create );
    puts("Keys init");
    return;
}

void threadDataDestructor(void* opaque)
{
    /* calling pthread_setspecific() here may have strange consequences */
    puts("Destructor");
    if(!key)
        puts("\tKey is already NULL");
    ThreadData *td = (ThreadData *)opaque;
    char *ptr = (char *) pthread_getspecific( *((pthread_key_t *)opaque));
    if(!ptr)
        return;
    printf("%d: %c - %s\n", td->id, td->c, ptr);
    fflush(stdout);
    free(ptr);
    ptr = NULL;
    return;
}

void setThreadData(ThreadData *td)
{
    char *mem = (char *)pthread_getspecific(key);
    const char *dt1 = "This is thread 1, my data is: foo";
    const char *dt2 = "This is thread 2, my data is: bar";
    if(!mem)
    {
        printf("\tthread %d: no data returned\n", td->id);
        return;
    }
    if(td->id)
        //strlen excludes '\0' but the allocated memory has been zeroized at
        //the beginning so it does not matter
        memcpy(mem, dt1, strlen(dt1) );
    else
    {
        memcpy(mem, dt2, strlen(dt2) );
        sleep(1);                                //just to exploit barriers
    }
}

void threadDataAllocator(void)
{
    char *tmp= (char *)malloc(DATA_SZ);
    CHECKER(!tmp, , malloc);
    memset( tmp, 0, DATA_SZ);
    pthread_setspecific(key, tmp);
}
