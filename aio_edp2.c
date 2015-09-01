/*
 * This is variant of aio_edp.c, with a following change:
 * - in setAioBuffer aio_sigevent.sigev_value will be set to the pointer to
 *   some custom value,
 * - setSigHandler() -> setSigAction(): sigaction() will have SA_SIGINFO flag
 *   that will cause that value passed to the handler inside siginfo_t,
 * - handler setup will need small reconstruction, instead of using sa_handler
 *   it will use sa_sigaction in order to take more arguments that will be
 *   passed,
 * - that enforces also a change in sigIoHandler -> sigIoAction
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <aio.h>

#define CHECKER( invok, cond, label ) do{                        \
    if( invok cond ){  perror(#label); exit(EXIT_FAILURE); }     \
    }while(0)

static struct termios oldT;
static void setSigAction(struct sigaction *sigAct, int sigNum,
        void (*actPtr)(int, siginfo_t *, void* ));
static void setAlrmHandler(struct sigaction *sigAct, int sigNum,
        void (*hPtr)(int));
static void sigIoAction(int sigNum, siginfo_t *info, void *uctx);
static void alrmHandler(int sigNum);
static void setTimer(int name, long initVal, long interVal);
static void setAioBuffer(struct aiocb *buff);
static void readAioBuffer(struct aiocb *buff);

static volatile sig_atomic_t inputFlag=0;
static bool loopFlag = true;
static const char* testStr="--test string--";

int main(int argc, char **argv)
{
    struct sigaction sigHandlers;
    struct aiocb aioBuff;
    sigset_t oldMask, pauseMask;

    memset(&sigHandlers, 0, sizeof(sigHandlers));
    memset(&aioBuff, 0, sizeof(aioBuff));

    /* setting SIGIO handler function */
    setSigAction(&sigHandlers, SIGIO, sigIoAction);
    /* blocking SIGIO */
    sigemptyset(&pauseMask);
    sigaddset(&pauseMask, SIGIO);
    sigprocmask(SIG_BLOCK, &pauseMask, &oldMask);
    /* now keyboard reading may be turned on */
    setAioBuffer(&aioBuff);
    aio_read(&aioBuff);
    setAlrmHandler(&sigHandlers, SIGALRM, alrmHandler);
    /* setting interval alarm that will wakeup the process each time */
    setTimer(ITIMER_REAL, 3000, 1000);
    while(loopFlag)
    {
        if(inputFlag)
        {
            inputFlag=0;
            readAioBuffer(&aioBuff);
        }
        else
        {
            puts("--down--"); fflush(stdout);
            //wakes up on SIGALRM
            sigsuspend(&oldMask);
            puts("--up--"); fflush(stdout);
        }
    }
    puts("--exit--"); fflush(stdout);
    //sigprocmask(SIG_UNBLOCK, &pauseMask, NULL);
    exit(0);
}

void setAioBuffer(struct aiocb *buff)
{
    static char c;
    /* setting aio_getchar */
    buff->aio_fildes = STDIN_FILENO;
    buff->aio_buf = &c;
    buff->aio_nbytes = 1;
    buff->aio_offset = 0;
    //what to do once the data are ready
    buff->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    buff->aio_sigevent.sigev_signo = SIGIO;
    //XXX: here is the value passed to sigIoAction()
    buff->aio_sigevent.sigev_value = (union sigval){ .sival_ptr = (void *)testStr };
}


void setSigAction(struct sigaction *sigAct, int sigNum,
        void (*actPtr)(int, siginfo_t *, void* uctx))
{
    //SIGIO handler
    //sa_mask - signals that should be blocked during handler execution
    //  + signal that trigerred the handler will be blocked
    sigemptyset(&sigAct->sa_mask);
    sigAct->sa_sigaction = actPtr;
    sigAct->sa_flags = SA_RESTART | SA_SIGINFO;
    CHECKER( sigaction(sigNum, sigAct, NULL), <0, setSigHandler );
    //alarm handler
}

void setAlrmHandler(struct sigaction *sigAct, int sigNum, void (*hPtr)(int))
{
    sigAct->sa_handler = hPtr;
    // sa_flags remain the same
    // during execution of alarmHandler SIGIO handler needs to be blocked
    // in order to keep the memory consistent
    sigemptyset(&sigAct->sa_mask);
    sigaddset(&sigAct->sa_mask, SIGIO);
    // reset flags to SA_RESTART because previously sa_sigaction has been used
    // whereas now is back sa_handler
    sigAct->sa_flags = SA_RESTART;
    CHECKER( sigaction(sigNum, sigAct, NULL), <0, alrmHandler );
}

static void sigIoAction(int sigNum, siginfo_t *info, void *uctx)
{
    /* nothing happens so much here, only the flag is set */
    inputFlag = 1;
    printf("value passed: %s\n", (char *)info->si_value.sival_ptr);
}

void alrmHandler(int sigNum)
{
    puts("****");
}

void setTimer(int name, long initVal, long interVal)
{
    /* initVal and interVal are in msecs */
    struct itimerval itv;
    long tmp;
    tmp = initVal/1000;
    itv.it_value.tv_sec = tmp;
    itv.it_value.tv_usec = initVal - (tmp*1000);
    tmp = interVal/1000;
    itv.it_interval.tv_sec = tmp;
    itv.it_interval.tv_usec = interVal - (tmp*1000);
    setitimer(name, &itv, NULL);
}


void readAioBuffer(struct aiocb *buff)
{
    CHECKER( aio_error(buff), , aio_read);
    char *outPtr = buff->aio_buf;
    //nb of bytes read
    if( aio_return(buff) == 1)
    {
        if(*outPtr == 'q')
            loopFlag = 0;
        else
            printf("-> %c\n", *outPtr);
    }
    //XXX: place new request
    aio_read(buff);
}

__attribute__((constructor)) void unsetEnter()
{
     struct termios newT;
     tcgetattr(STDIN_FILENO, &oldT);
     newT = oldT;
     //deassociate input and output, otherwise each character appear twice
     newT.c_lflag &= ~(ICANON | ECHO );
     tcsetattr( STDIN_FILENO, TCSANOW, &newT);
}

__attribute__((destructor)) void setEnter()
{
    tcsetattr( STDIN_FILENO, TCSANOW, &oldT);
}

