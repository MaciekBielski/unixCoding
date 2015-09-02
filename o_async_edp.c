/* Topic: dealing asynchronous events with O_ASYNC flag of a file descritor
 *
 * This program presents handling asynchronous events with following approach:
 * - event source is defined by a file descriptor [here: STDIN_FILENO]
 * - process responsible for handling with SIGIO and SIGURG on that file
 *   descriptor is specified: fcntl()
 * - access mode of the file descroptor is extended with O_ASYNC
 * - SIGIO handler is set
 * - SIGALRM handler is set but with one reservation, that SIGIO is blocked
 *   during sigAlrmHandler is operating
 * - 'screen refreshing' is done periodically, whereas SIGIO is handled
 *   immediately as it happens
 * - alarma is set, program goes to sleep and only SIGALRM can wakeup it
 *
 * Process is informed about an event once the data are in kernel buffers.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>

#define CHECKER( invok, cond, label ) do{                        \
    if( invok cond ){  perror(#label); exit(EXIT_FAILURE); }     \
    }while(0)

static struct termios oldT;
static void setSigHandler(struct sigaction* sigAct, int sigNum,
        void (*hPtr)(int));
static void setAlrmHandler(struct sigaction *sigAct, int sigNum,
        void (*hPtr)(int));
static void sigIoHandler(int sigNum);
static void alrmHandler(int sigNum);
static int setTimer(int name, long initVal, long interVal);

int main(int argc, char **argv)
{
    int oldFlags;
    struct sigaction sigHandlers;
    memset(&sigHandlers, 0, sizeof(sigHandlers));
    /* setting file descriptor event's (SIGIO) handler process */
    CHECKER( fcntl(STDIN_FILENO, F_SETOWN, getpid() ), <0, setown );
    CHECKER( (oldFlags=fcntl(STDIN_FILENO, F_GETFL)), <0, getfl) ;
    /* O_ASYNC approach */
    CHECKER( fcntl(STDIN_FILENO, F_SETFL, (oldFlags|O_ASYNC) ), <0, setfl);
    /* setting file descriptor event's (SIGIO) handler function */
    setSigHandler(&sigHandlers, SIGIO, sigIoHandler);
    setAlrmHandler(&sigHandlers, SIGALRM, alrmHandler);
    /* setting interval alarm that will wakeup the process each time */
    setTimer(ITIMER_REAL, 4000, 1000);
    while(1)
    {
        pause();
        printf("-pause-");fflush(stdout);
    }
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

void setAlrmHandler(struct sigaction *sigAct, int sigNum, void (*hPtr)(int))
{
    sigAct->sa_handler = hPtr;
    //sa_flags remain the same
    //during execution of alarmHandler SIGIO handler needs to be blocked
    //in order to keep the memory consistent
    sigemptyset(&sigAct->sa_mask);
    sigaddset(&sigAct->sa_mask, SIGIO);
    /* XXX: is it possible to obtain signals previously set? */
    CHECKER( sigaction(sigNum, sigAct, NULL), <0, alrmHandler );
    return;
}

void setSigHandler(struct sigaction *sigAct, int sigNum, void (*hPtr)(int))
{
    //SIGIO handler
    //sa_mask - signals that should be blocked during handler execution
    //  + signal that trigerred the handler will be blocked
    sigemptyset(&sigAct->sa_mask);
    /* alternatively sa_sigaction may be used with more information */
    sigAct->sa_handler = hPtr;
    sigAct->sa_flags = SA_RESTART;
    CHECKER( sigaction(sigNum, sigAct, NULL), <0, setSigHandler );
    //alarm handler
}

static void sigIoHandler(int sigNum)
{
    /* this getch() will return immediately since the character is already in
     * kernel's buffer */
    char c;
    CHECKER( (c=getchar()), ==EOF, getchar);
    //c=getchar();
    if(c=='q')
        exit(0);
    else
        printf("-> %c\n",c);
}

void alrmHandler(int sigNum)
{
    puts("====***====");
}

int setTimer(int name, long initVal, long interVal)
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
