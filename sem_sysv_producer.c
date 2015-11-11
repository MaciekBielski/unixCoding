/* Producer consumer problem extended with taking-turns behavior */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

#define CWD_SZ 256

//from sem_sysv_common.c
int semInitialize(key_t semKey, unsigned short* initVals, int semNb);
void setSemVal(int setId, int *semIndices, int *opVals, int *flags, int changesNb);

int main(int argc, char** argv)
{
    char *child_args[]={NULL, NULL};
    char filepath[CWD_SZ];
    int status, semSetId;
    unsigned short initVals[2];
    pid_t pid;
    key_t semKey;

    getcwd(filepath,CWD_SZ);
    strcat(filepath,"/consumer_sysv");
    child_args[0] = filepath;
    //creation of uninitialized semaphore - not an atomic process
    semKey = ftok(filepath, '?'); //alternatively could be IPC_PRIVATE
    //initialize two semaphores, consumer should wait for it
    // 0 - when consumer can continue
    // 1 - when producer can continue
    // thanks to that they can take turns - desired behaviour
    initVals[0] = 0;
    initVals[1] = 1;
    semSetId = semInitialize(semKey, initVals, 2);
    pid=fork();
    if(!pid) //child process
    {
        execvp(child_args[0],child_args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stdout,"--INIT--\n"); fflush(stdout);
        int semId, opVal, flag;
        for(int i=0; i<5; i++)
        {
            //it should pass for the first time
            semId = 1;
            opVal = -1;
            flag = 0;
            setSemVal(semSetId, &semId, &opVal, &flag, 1);
            fprintf(stdout,"--produced--\n"); fflush(stdout);
            sleep(1);
            //pass when producer can continue
            semId = 0;
            opVal = 1;
            flag = 0;
            setSemVal(semSetId, &semId, &opVal, &flag, 1);
        }
        wait( &status );
        //TODO: removing semaphore set
        if( semctl(semSetId, 888, IPC_RMID) <0)
            perror("Was consumer still waiting?\n\t");
    }
    // return 0;
}

