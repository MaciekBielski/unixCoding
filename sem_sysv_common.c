#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdlib.h>

typedef union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
} SemUn;

int semInitialize(key_t semKey, unsigned short* initVals, int semNb)
{
/* initializes the only one semaphore used to 1, allocation and removing must
 * be done by the same process, returns id of set of semaphores */
    int outId;
    SemUn semUnObj;
    outId=semget(semKey, semNb, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
    if( outId <0 )
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    //initializing semaphore - setting our semaphores to 1
    semUnObj.array = initVals;
    if( semctl(outId, 8888, SETALL, semUnObj) <0 )
    {
        perror("semctl: init");
        exit(EXIT_FAILURE);
    }
    return outId;
}

/* semSetId indicates the set of semaphores, sOps points to an array of nSops
 * elements, each of them is an element of struct sembuf with fields:
   - unsigned short sem_num; -> sem number
   - short          sem_op;  -> operation (change value)
   - short          sem_flg; -> operation flag
* if sem_op =0 then the semaphore blocks until it's value will become 0,
* if sem_op >0 then it's immediately incremented by that value
* if sem_op <0 then it blocks until semval will reach (sem_op-semval)
* All operations are performed atomically, more detailed caises: SEM_UNDO, signals
* On success sempid of a semaphore is set to caller's PID.
*/
void setSemVal(int setId, int *semIndices, int *opVals, int *flags, int changesNb)
{
    /* preparing the atomic changes */
    struct sembuf semOps[changesNb];
    for(int i=0; i<changesNb; i++)
    {
        semOps[i].sem_num = semIndices[i];
        semOps[i].sem_op = opVals[i];
        semOps[i].sem_flg = flags[i];
    }
    //XXX: the third parameter is number of semaphores to be set in an atomic
    //operation, not the number of all parameters
    if( semop(setId, semOps, 1) <0 )
    {   
        perror("semop: wait");
        exit(EXIT_FAILURE);
    }
}
