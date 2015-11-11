/* Producer consumer problem extended with taking-turns behavior */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/types.h>        /* For mode constants */
#include <sys/wait.h>        /* For mode constants */
#include <semaphore.h>


#define CWD_SZ 256

int main(int argc, char** argv)
{
    char *child_args[]={NULL, NULL};
    char filepath[CWD_SZ];
    const char *semProdNm="semProducer", *semConsNm="semConsumer";
    int status;
    pid_t pid;
    sem_t *semProd, *semCons;

    getcwd(filepath,CWD_SZ);
    strcat(filepath,"/consumer_posix");
    child_args[0] = filepath;
    //creation of uninitialized semaphore - not an atomic process
    //initialize two semaphores, consumer should wait for it
    // 0 - when consumer can continue
    // 1 - when producer can continue
    // thanks to that they can take turns - desired behaviour
    //sem_open();
    //sem_post();
    //sem_wait();
    //sem_close();
    //sem_overview
    //last argument is an initial value
    semProd = sem_open(semProdNm, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 1);
    semCons = sem_open(semConsNm, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 0);
    if( semProd==SEM_FAILED || semCons==SEM_FAILED )
    {
        perror("producer: sem_open");
        exit(EXIT_FAILURE);
    }
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
        for(int i=0; i<5; i++)
        {
            //it should pass for the first time
            sem_wait(semProd);
            fprintf(stdout,"--produced--\n"); fflush(stdout);
            sleep(1);
            //pass when producer can continue
            sem_post(semCons);
        }
        wait( &status );
        //TODO: removing semaphore set
        if( sem_close(semProd)<0 || sem_close(semCons)<0)
            perror("Was consumer still waiting?\n\t");
        else
        {
            sem_unlink(semProdNm);
            sem_unlink(semConsNm);
        }
    }
    // return 0;
}

