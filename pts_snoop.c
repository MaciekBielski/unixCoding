/* Program opens a pseudo-terminal device of a number provided
 * as an argument and waits for asynchronous events on it
 * It can write or read from the teminal.
 * It has been created to read/send characters to QEMU via /dev/pts/X
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#define BUFF_SZ 1
#define BUFF_SZ_IN 1
#define PATH_SZ 16

#define CHECKER( fun, cond, err ) do{ \
    if( (fun) cond ){ perror(#err); exit(EXIT_FAILURE); } \
}while(0)

static inline void usage(const char *s)
{
    printf("Usage: %s <pts_nb>\n", s);
}

static void readDescriptor(int fd);
static void writeToDescriptor(int fd);

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        usage( argv[0]);
        exit(EXIT_FAILURE);
    }
    //setting the correct pts path
    char ptsPath[PATH_SZ]; 
    const char* pathBase = "/dev/pts/";
    memset(ptsPath, NULL, PATH_SZ);
    memcpy(ptsPath, pathBase, strlen(pathBase));
    strncat(ptsPath,argv[1],4);
    //opening epoll
    int epfd, ret;
    epfd = epoll_create1(NULL);
    CHECKER( epfd, ==-1, epoll_create);
    // opening the pts file descriptor
    int ptsFd = open(ptsPath, O_RDWR|O_NOCTTY);
    CHECKER( ptsFd, <0, opening_pts);
    // setting up the event
    struct epoll_event evOut;
    struct epoll_event evIn[2];
    evIn[0]= (struct epoll_event){
        .events = EPOLLIN,
        .data = { .fd=ptsFd },
    };
    evIn[1] = (struct epoll_event){
        .events = EPOLLIN,
        .data = { .fd=STDIN_FILENO },
    };
    //setting up poll for the events
    CHECKER( epoll_ctl(epfd, EPOLL_CTL_ADD, ptsFd, evIn), ==-1, epoll_ctl_pts);
    CHECKER( epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, evIn+1), ==-1, epoll_ctl_stdin);
    // main loop
    printf("> "); fflush(stdout);
    for(;;)
    {   //block until an event occurs
        ret=epoll_wait(epfd, &evOut, 1, 0);
        if(ret<0)
        {
            if(errno==EINTR)
                continue;
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        if(ret>0)
        { //there is descriptor placed
            if( evOut.events & EPOLLIN)
            {
                if(evOut.data.fd==ptsFd)
                    readDescriptor(ptsFd);
                if(evOut.data.fd==STDIN_FILENO)
                    writeToDescriptor(ptsFd);
            }
            printf("> "); fflush(stdout);
        }
        usleep(250000);
    }
    close(epfd);
    close(ptsFd);
}

static void writeToDescriptor(int fd)
{
    char buff[512] = {0};
    size_t i=0;
    for(; i<512; i++)
    {
        char tmp=getchar();
        printf("%02x,", tmp); fflush(stdout);
        if( tmp==0x0a )
            break;
        buff[i] = tmp;
    }
    printf("%d\n",i);
    if(i>0)
    {
        size_t nbytes=0;
        printf("\rsent: %s\n", buff); fflush(stdout);
        CHECKER( (nbytes=write(fd, buff, i)), ==-1, write_to_device);
        printf("nbytes: %d\n",nbytes);
    }
}

static void readDescriptor(int fd)
{
    char buff[BUFF_SZ];
    int ret=0;
    memset(buff, '\0', BUFF_SZ);
    ret = read(fd, buff, BUFF_SZ);
    CHECKER( ret, <0, read);
    puts("");
    fflush(stdout);
    printf("ret:%d\n ", ret);
    for(int i=0; ret && i<ret; i++)
        printf("%02x ",buff[i]);
    puts("");
    fflush(stdout);
}
