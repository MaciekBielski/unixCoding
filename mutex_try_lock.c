#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

char* pseudoList[] = {"aa","ab","ac","ad","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",
                    "aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",                      
                    "aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",
                    "aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",
                    "aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",
                    "aa","bb","cc","dd","ee","ff","gg", "hh", "ii", "jj","kk", "ll","mm","nn","oo",
                     NULL};
char** plPtr = pseudoList;
pthread_mutex_t pseudoListMutex;

void* threadFun(void *p){
    int *pp = (int*) p;
    
    for(;;){
        int lockFlag=5;
        //try-lock mutex
        lockFlag=pthread_mutex_trylock(&pseudoListMutex);
        if(lockFlag==0){
            char* currentWord = *plPtr;
            if(currentWord!=NULL)
                plPtr++;
            //unlock mutex
            pthread_mutex_unlock(&pseudoListMutex);
            if(currentWord==NULL)
                break;
            printf("%d: %s\t", *pp, currentWord);
            sleep(2.5L);
        }
        else if(lockFlag!=0){
            printf("\tthread %d is busy\n", *pp);
        }
    }
    return NULL;
}

int main(){
    static const int threadNb = 100;
    int tids[threadNb] ;

    pthread_t threads[threadNb];
    //mutex initialization with non-default attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setkind_np(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&pseudoListMutex, &attr);
    pthread_mutexattr_destroy(&attr);
    for(int i=0; i<threadNb; i++){
        tids[i] = i;
        pthread_create(&threads[i], NULL, &threadFun, &tids[i]);
    }
    for(int i=0; i<threadNb; i++)
        pthread_join(threads[i], NULL);

    return 0;
}

