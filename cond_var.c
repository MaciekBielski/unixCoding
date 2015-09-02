/* Topic: pthread conditional variables
 *
 * man pthread_cond_wait
 * A condition variable blocks upon itself and releases the mutex atomically.
 * The mutex should be locked by the calling thread before.
 * If after that another thread is able to acquire it then it can safely invoke
 * pthread_cond_signal() or pthread_cond_broadcast() and it will reach a
 * correct destination.
 * Upon return from a pthread_cond_wait() a calling thread owns the mutex again
 * and it's locked.
 *
 * The boolean predicate should be re-tested after wakeup since spurious
 * wakeups are possible- flag is checked in the loop.
 *
 * If the scheduling behavior has to be predictable then pthread_cond_signal()
 * should be followed by pthread_mutex_unlock() - in that order.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

pthread_cond_t cvFrom1,cvFrom2 ;
pthread_mutex_t mutex;
bool flag=false;

void* threadFun1(void *p)
{
    for(int i=0; i<5; i++){
        pthread_mutex_lock(&mutex);
        if(!flag){
            puts("===thread1 waits===");
            pthread_cond_wait(&cvFrom2, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        puts("\t---thread1 executes--");
        sleep(1.0L);
        pthread_mutex_lock(&mutex);
        flag = !flag;
        //mutex_lock
        pthread_cond_signal(&cvFrom1);
        //mutex_unlock
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* threadFun2(void *p)
{
    for(int i=0; i<5; i++){
        pthread_mutex_lock(&mutex);
        if(flag){
            puts("===thread2 waits===");
            pthread_cond_wait(&cvFrom1, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        puts("\t---thread2 executes--");
        sleep(1.0L);
        pthread_mutex_lock(&mutex);
        flag = !flag;
        //mutex_lock
        pthread_cond_signal(&cvFrom2);
        //mutex_unlock
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(){
    const int threadNb = 2;
    pthread_t tid[threadNb];
    
    // initialization
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cvFrom1, NULL);
    pthread_cond_init(&cvFrom2, NULL);
    //(void*) funs[] - would be casting
    void* (funs[]) = {&threadFun1, &threadFun2};

    puts("\nSTART");
    sleep(1);
    for(int i=0; i<threadNb; i++, sleep(1.0L))
        pthread_create(&tid[i], NULL, funs[i], NULL);
    for(int i=0; i<threadNb; i++)
        pthread_join(tid[i], NULL);
    puts("\nSTOP");
    return 0;
}
