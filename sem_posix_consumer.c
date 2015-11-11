#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#define CWD_SZ 256

int main(int argc, char** argv)
{
    //for the same filepath and the same 8-bit value ftok will create the same
    //key value as for parent process - no need to transfer as a char
    // indices:
    // 0 - when consumer can continue
    // 1 - when producer can continue
    char filepath[CWD_SZ];
    const char *semProdNm="semProducer", *semConsNm="semConsumer";
    sem_t *semProd, *semCons;

    getcwd(filepath,CWD_SZ);
    strcat(filepath,"/consumer_sysv");
    //get the semaphores allocated and initialized by parent process
    semProd = sem_open(semProdNm, 0);
    semCons = sem_open(semConsNm, 0);
    if( semProd==SEM_FAILED || semCons==SEM_FAILED )
    {
        perror("consumer: sem_open");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    for(int i=0;i<5;i++)
    {
        // wait for producer
        sem_wait(semCons);
        fprintf(stdout,"--consumed--\n"); fflush(stdout);
        sleep(1);
        sem_post(semProd);
    }
    return 0;
}

