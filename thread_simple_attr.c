#include<stdio.h>
#include<pthread.h>

typedef struct charPrintParams{
    char character;
    int count;
    char* result;
}charPrintParams;

void* printChar(void *parameters){
    printf("\n---Starting thread: %u---\n", pthread_self());
    charPrintParams *p = (charPrintParams*) parameters;
    for(int i=0; i<p->count; ++i)
        fputc(p->character, stderr);
    return p->result;
}

int main(){
    charPrintParams tid1Args={.character='a', .count=700, .result="threadA"};
    charPrintParams tid2Args={.character='b', .count=300, .result="threadB"};
    pthread_t tid1, tid2;
    
    char* res1=0;
    /* thread's attribute object declaration */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid1, NULL, &printChar, &tid1Args);
    pthread_create(&tid2, &attr, &printChar, &tid2Args);
    pthread_join(tid1, (void*)&res1);
    //second thread is detached, cannot be joined
    printf("\njoinable thread finished with values: \n\t%s\n", res1);

    return 0;
}
