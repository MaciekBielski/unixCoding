#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

char* pseudoList[] = {"aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn", NULL};
char** plPtr = pseudoList;
pthread_mutex_t pseudoListMutex;

void* threadFun(void *p){
    int *pp = (int*) p;
    
    for(;;){
        //lock mutex
        pthread_mutex_lock(&pseudoListMutex);
        char* currentWord = *plPtr;
        if(currentWord!=NULL)
            plPtr++;
        //unlock mutex
        pthread_mutex_unlock(&pseudoListMutex);
        if(currentWord==NULL)
            break;
        printf("%d: %s\n", *pp, currentWord);
        sleep(2.5L);
    }
    return NULL;
}

int main(){
    static const int threadNb = 5;
    static int tids[] = {1,2,3,4,5};
    pthread_t threads[threadNb];
    //mutex initialization with non-default attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&pseudoListMutex, &attr);
    pthread_mutexattr_destroy(&attr);
    for(int i=0; i<threadNb; i++)
        pthread_create(&threads[i], NULL, &threadFun, &tids[i]);
    for(int i=0; i<threadNb; i++)
        pthread_join(threads[i], NULL);

    return 0;
}

