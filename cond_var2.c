#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

/* Test: Is the mutex mCv locked after waiting function in thread1 returns?
 * ANSWER: No, it leaves unlocked from beginning of waiting
 */
pthread_cond_t cv;
pthread_mutex_t m,mCv,postWait;
bool flag=false;

void* threadFun1(void *p){
    pthread_mutex_lock(&m);
    flag = true;
    pthread_mutex_lock(&mCv);
    printf("thread1 waiting\n");
    pthread_mutex_unlock(&m);
    pthread_mutex_lock(&postWait);
    pthread_cond_wait(&cv,&mCv);
    sleep(2.0);
    puts("postWait is probably locked after return from waiting");
    pthread_mutex_unlock(&postWait);
    //is mCv locked here?
    return NULL;
}

void* threadFun2(void *p){
    for(;;){
        pthread_mutex_lock(&m);
        if(flag)
            break;
        pthread_mutex_unlock(&m);
    }
    printf("thread2 unlocked\n");
    pthread_mutex_unlock(&m);
    //mutex_lock
    pthread_cond_signal(&cv);
    //mutex_unlock
    pthread_mutex_lock(&postWait);
    printf("postWait was unlocked after return from waiting\n");
    pthread_mutex_unlock(&postWait);
    return NULL;
}

int main(){
    const int threadNb = 2;
    pthread_t tid[threadNb];
    
    // initialization
    pthread_mutex_init(&m, NULL);
    pthread_mutex_init(&mCv, NULL);
    pthread_mutex_init(&postWait, NULL);
    pthread_cond_init(&cv, NULL);
    
    void* (funs[]) = {&threadFun1, &threadFun2};

    puts("\n\nSTART============");
    sleep(1.9);
    for(int i=0; i<threadNb; i++)
        pthread_create(&tid[i], NULL, funs[i], NULL);
    for(int i=0; i<threadNb; i++)
        pthread_join(tid[i], NULL);
    puts("\n\nSTOP");
    return 0;
}
