#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t sem;

void* fun1 (void* p){
    sleep(1.0);
    puts("fun1");
    sem_post(&sem);
    return NULL; 
}
void* fun2 (void* p){
    sleep(2.0);
    puts("fun2");
    sem_post(&sem);
    return NULL; 
}

int main(){
    void* (*funs[])(void *p)={&fun1,&fun2};
    pthread_t tid[2];
    sem_init(&sem,0,0);
    for(size_t t=0; t<2; t++)
        pthread_create(&tid[t], NULL, funs[t],NULL);
    sem_wait(&sem);
    puts("first");
    sem_wait(&sem);
    puts("second");

    for(size_t t=0; t<2; t++)
        pthread_join(tid[t],NULL);
    sem_destroy(&sem);
}
