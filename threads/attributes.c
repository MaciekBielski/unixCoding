/* Topic: setting thread attributes 
 *
 * Simple example shows how to set thread attributes. In that case one thread
 * is created as detached - it's not joinable. After waiting for the other
 * thread program will exit.
 * 
 * To observe the effect comment sleep() at the end.
 * pthread_self() - to get the ID of a calling thread;
 */
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

typedef struct Params{
    char character;
    int count;
    char* result;
}Params;

void* printChar(void *parameters){
    fprintf(stderr, "---Starting thread: %d---\n", pthread_self());
    Params *p = (Params*) parameters;
    sleep(300/p->count);            // just poor pseudo-synchronisation
    for(int i=0; i<p->count; ++i)
        fputc(p->character, stderr);
    return p->result;
}

int main(){
    Params tid1Args={.character='a', .count=300, .result="threadA"};
    Params tid2Args={.character='b', .count=100, .result="threadB"};
    pthread_t tid1, tid2;
    
    char* res1=0;
    /* thread's attribute object setup*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // creatig joinable thread
    pthread_create(&tid1, NULL, &printChar, &tid1Args);
    // creating detached thread
    pthread_create(&tid2, &attr, &printChar, &tid2Args);
    pthread_join(tid1, (void*)&res1);
    //second thread is detached, cannot be joined
    printf("\nJoinable thread finished with value: \n\t%s\n", res1);
    //without this 'bbbbb...' wouldn't be printed at all perhaps
    sleep(3);
    return 0;
}
