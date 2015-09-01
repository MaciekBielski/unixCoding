#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct tranParams{
    int *accounts;
    int srcId;
    int dstId; 
    int sum;
}tranParams;

void* cashTransaction(void* params){
    int oldState=0;
    tranParams *p = (tranParams*) params;
    /*Begin critical section*/
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState);
    puts("--start!--");
    p->accounts[p->srcId] -= p->sum;
    p->accounts[p->dstId] += p->sum;   
    sleep(2);
    puts("--done!--");
    pthread_setcancelstate(oldState, NULL);
    //end is necessarily, otherwise function can return even before cancellation
    sleep(1);
    puts("--end--");
    /*End critical section*/
    return NULL;
}

int main(){
    static int accs[10];
    for(int i=0; i<10; i++)
        accs[i] = 300;
    pthread_t tid1;
    tranParams tranArgs1={.accounts=accs, .srcId=0, .dstId=9, .sum=200};
    pthread_create(&tid1, NULL, &cashTransaction, &tranArgs1);
    //trying to cancel a thread
    pthread_cancel(tid1);
    pthread_join(tid1, NULL);
    
    return 0;
}

