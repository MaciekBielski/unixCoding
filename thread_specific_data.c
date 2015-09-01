#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>

static pthread_key_t threadSpecKey;

void* threadCleanup(void* p){
    int *pp = (int*)p;
    printf("Cleanup handler for a thread: %d\n", *pp);
    return NULL;
}

void* threadFun(void* params){
    printf("\n--Starting thread: %u --\n", pthread_self()); 
    //accessing a thread-specific value by a common key
    pthread_setspecific(threadSpecKey, params);
    //registering cleanup function for that thread
    pthread_cleanup_push(threadCleanup, params);
    sleep(1.0L);
    puts("\t--thread's done--");
    //unregistering and calling cleanup function
    pthread_cleanup_pop(1);
    return NULL;
}

void cleanupThreadSpec(void* value){
    int *p = (int*) value;
    printf("Cleanup function for thread specific value: %d\n", *p);
    //releasing memory allocated for that specific value
    free(value);
}

int main(){
    pthread_t threads[5];
    //creating a key for thread-specific values
    pthread_key_create(&threadSpecKey, cleanupThreadSpec);
    for (int i=0; i<5; i++){
        int *tmp = malloc(sizeof(i));
        *tmp = i;
        pthread_create(&threads[i], NULL, &threadFun, tmp);
    }
    for (int i=0; i<5; i++)
        pthread_join(threads[i], NULL);
    
    return 0;
}
