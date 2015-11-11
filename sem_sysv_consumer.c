#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


#define CWD_SZ 256

void setSemVal(int setId, int *semIndices, int *opVals, int *flags, int changesNb);

int main(int argc, char** argv)
{
    //for the same filepath and the same 8-bit value ftok will create the same
    //key value as for parent process - no need to transfer as a char
    // indices:
    // 0 - when consumer can continue
    // 1 - when producer can continue
    char filepath[CWD_SZ];
    key_t semKey;
    int semSetId;

    getcwd(filepath,CWD_SZ);
    strcat(filepath,"/consumer_sysv");
    semKey = ftok(filepath, '?'); //alternatively could be IPC_PRIVATE
    //get the semaphores allocated and initialized by parent process
    semSetId = semget(semKey, 2, 0);
    sleep(1);
    int semId, opVal, flag;
    for(int i=0;i<5;i++)
    {
        // wait for producer
        semId = 0;
        opVal = -1;
        flag = 0;
        setSemVal(semSetId, &semId, &opVal, &flag, 1);
        fprintf(stdout,"--consumed--\n"); fflush(stdout);
        sleep(1);
        semId = 1;
        opVal = 1;
        flag = 0;
        setSemVal(semSetId, &semId, &opVal, &flag, 1);
    }
    return 0;
}

