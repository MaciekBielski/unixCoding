/* Topic: synchronizing threads with POSIX semaphores
 *
 * POSIX semaphores can synchronised both threads and processes, depending how
 * they are initialized.
 * help: man sem_overview
 *
 * sem_post() increments semaphore, sem_wait() decrements it but if it's
 * already zero then it blocks until the value becomes larger.
 *
 * - named semaphore - more convenient for two processes, no need to pass a
 *   semaphore address as execve argument:
 *   sem_open() -> [post/wait] -> sem_close() -> sem_unlink()
 *   Located in kernel space (under /dev/shm).
 *
 * - unnamed semaphore - must be placed in a shared memory, OK for threads,
 *   more awkward for processes:
 *   sem_init() -> [post/wait] -> sem_destroy()
 *   Located in memory.
 *
 * About the program:
 * A second argument of sem_init() is 0- semaphore for threads of a process.
 * Third argument - initial value.
 * First both functions needs to run, fun1 -> fun2 -> master thread can go.
 */
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

static sem_t semX, semY;

void* fun1 (void* p){
    puts("--fun1--");
    sleep(1.5);
    sem_post(&semX);
    return NULL; 
}
void* fun2 (void* p){
    sem_wait(&semX);
    puts("--fun2--");
    sleep(0.5);
    sem_post(&semY);
    sleep(2);
    sem_post(&semX);
    return NULL; 
}

int main(){
    void* (*funs[])(void *p)={&fun1,&fun2};
    pthread_t tid[2];
    sem_init(&semX,0,0);
    for(size_t t=0; t<2; t++)
        pthread_create(&tid[t], NULL, funs[t],NULL);
    // master thread is waiting here till both functions will finish
    sem_wait(&semY);
    puts("--first checkpoint--");
    sem_wait(&semX);
    puts("--second checkpoint--");

    for(size_t t=0; t<2; t++)
        pthread_join(tid[t],NULL);
    sem_destroy(&semX);
    sem_destroy(&semY);
}
