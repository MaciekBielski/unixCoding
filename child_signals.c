#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define CWD_SZ 256
#define SEG_SZ 0x1000

char *seg_ptr=NULL;
int seg_id;
pid_t childPid;

static void sigusr1_handler(int sigNum)
{
    fprintf(stderr,"Value: %s\n",seg_ptr);
}

static void sigchld_handler(int sigNum)
{
    //shared memory clean-up
    shmdt(seg_id);
    seg_ptr=NULL;
    shmctl(seg_id, IPC_RMID, 0);
    fprintf(stderr,"Child finish\n");
}

static void sigterm_handler(int sigNum)
{
    kill(childPid, SIGKILL);
    fprintf(stderr,"SIGTERM\n");
    sleep(1);
    exit(EXIT_SUCCESS);
}

static void setSigHandler(int sigNum, void (*hPtr)(int));

int main(int argc, char** argv)
{
    char *child_args[]={NULL, NULL, NULL};
    char filepath[CWD_SZ], seg_id_str[8];
    int status;
    pid_t pid;
    //shared memory variables

    getcwd(filepath,CWD_SZ);
    strcat(filepath,"/server");
    child_args[0] = filepath;
    //shared memory setup
    seg_id = shmget(IPC_PRIVATE, SEG_SZ,
            IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
    sprintf(seg_id_str,"%d",seg_id);
    child_args[1] = seg_id_str;
    seg_ptr = (char *) shmat(seg_id, 0, SHM_RDONLY);
    //signals setup
    setSigHandler(SIGUSR1, sigusr1_handler);
    setSigHandler(SIGCHLD, sigchld_handler);
    setSigHandler(SIGTERM, sigterm_handler);
    pid=fork();
    if(pid)
        childPid=pid;
    if(!pid) //child process
    {
        execvp(child_args[0],child_args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        // //parent process - close writing descriptor
        waitpid(pid, &status, WNOHANG);
        for(;;)
        {
            fprintf(stdout,". ");
            fflush(stdout);
            sleep(1);
        }
        return 0;
    }
}

static void setSigHandler(int sigNum, void (*hPtr)(int))
{
    struct sigaction sigAct;
    //
    memset(&sigAct, 0, sizeof(sigAct));
    sigAct.sa_handler = hPtr;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags &= 0;
    sigAct.sa_flags |= SA_RESTART;
    if( sigaction(sigNum, &sigAct, NULL) <0 )
    {
        perror("setSigHandler");
        exit(EXIT_SUCCESS);
    }
}
