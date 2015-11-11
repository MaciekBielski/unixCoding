#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>

#define PORT_NB "4545"
#define DATA_SZ 4096
#define FIN_STR "__END__"
#define REQ_STR "__REQ__"

int bindSocket( int family, int socktype, int flags);
int acceptConnections(const int *servSock);
//from common.o
int recvLine(const int srcSock, char *buff, int buffSz);
int sendLine(int dstSock, char *buff, int buffSz);

//for socket options
int yes=1;

int main(int argc, char** argv)
{
    int serverSock, clientSock;
    struct sigaction sigAct;
    char data[DATA_SZ];
    //shared memory vars
    int segId;
    char *segPtr=NULL;
    
    if(argc !=2)
    {
        perror("Bad arguments");
        exit(EXIT_FAILURE);
    }
    //set up shared memory
    segId = atoi(argv[1]);
    segPtr = (char *) shmat(segId, 0, 0);
    //obtaining binded socket
    serverSock = bindSocket(AF_INET, SOCK_STREAM, AI_PASSIVE);
    //listening
    listen(serverSock, 1);
    while(1)
    {
        clientSock = acceptConnections(&serverSock);
        //receive data from client
        for(;;)
        {
            //server asks for a new data once it's ready
            sendLine(clientSock, REQ_STR, strlen(REQ_STR)+1);
            fprintf(stderr, "Server sent: %s\n", REQ_STR); fflush(stderr);
            recvLine(clientSock, segPtr, DATA_SZ);
            fprintf(stderr, "Server received: %s\n", segPtr); fflush(stderr);
            if( !strncmp(segPtr, FIN_STR, strlen(FIN_STR)+1) )
                break;
        }
        close(clientSock);
    }
    close(serverSock);
    //shared memory cleanup
    shmdt(&segId);
    segPtr=NULL;
    exit(EXIT_SUCCESS);
}

int acceptConnections(const int *servSock)
{
    int outSocket;
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrSz = sizeof(clientAddr);
    if( (outSocket = accept(
            *servSock,
            (struct sockaddr *)&clientAddr,
            &clientAddrSz
            )) ==-1)
    {
        perror("server: accept");
        exit(EXIT_FAILURE);
    }
    else
    {
    //    inet_ntop( clientAddr.ss_family,
    //        (struct sockaddr *)&clientAddr,
    //        buff,
    //        sizeof(buff)
    //        );
    //    fprintf(stdout, "Client from: %s\n", buff);
    }
    return outSocket; 
}

int bindSocket( int family, int socktype, int flags)
{
    struct addrinfo hints, *servInfo, *p;
    int outSocket, retVal;
    //hints setup
    memset(&hints, NULL, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    //IPv4 only
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    //getting linked list of addresses
    if( ( retVal = getaddrinfo(NULL, PORT_NB, &hints, &servInfo) ) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retVal));
        exit(EXIT_FAILURE);
    }
    //binding to the first good result
    for(p = servInfo; p!=NULL; p = p->ai_next)
    {
        //try to open a socket with a given address
        if( (outSocket =
            socket(p->ai_family, p->ai_socktype, p->ai_protocol) )
            == -1)
        {
            //perror("server: socket");
            continue;
        }
        //if it succeeded, set socket to reuse ports
        if( setsockopt(outSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
                ==-1 )
        {
            perror("setsoCkopt");
            exit(EXIT_FAILURE);
        }
        //bind the socket to a given port
        if( bind(outSocket, p->ai_addr, p->ai_addrlen) == -1 )
        {
            close(outSocket);
            perror("server: bind");
            exit(EXIT_FAILURE);
        }
        break;
    }
    //if no addrinfo object has been initialized
    if(p==NULL)
    {
        fprintf(stderr,"server: no addrinfo obtained");
        exit(EXIT_FAILURE);
    }
    //here a correctly open socket is binded - addrinfo resourced can be freed
    freeaddrinfo(servInfo);
    return outSocket;
}
