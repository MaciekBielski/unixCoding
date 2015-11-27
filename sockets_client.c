/* Data samples need to be provided line by line,
 * Each row can have the length of DATA_SZ-2 characters: [#DATA_SZ-2#|\n|\0]
 * - fgets read max DATA_SZ-1 (including \n) and appends \0
 * - '\n' is used to verify the end of a line and detect too long rows
 * All wrong rows will be dropped
 * After parsing the trailing '\n' is replaced by '\0'
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT_NB "4545"
#define DATA_SZ 4096
#define FIN_STR "__END__" 
#define REQ_STR "__REQ__"
#define REQ_SZ 8

int recvLine(const int srcSock, char *buff, int buffSz);
int sendLine(int dstSock, char *buff, int buffSz);

static inline void recvAskLine(const int srcSock, char *buff, int buffSz)
{
    do
    {
        recvLine(srcSock, buff, buffSz);
        fprintf(stderr, "Client received: %s\n", buff); fflush(stderr);
        if( !strncmp( buff, REQ_STR, strlen(REQ_STR)+1 ))
            break;    
        else
            break;    
    }
    while(1);
}

int main(int argc, char** argv)
{
    int clientSock, retVal, last;
    struct addrinfo hints, *clientInfo, *p;
    char buff[INET_ADDRSTRLEN], data[DATA_SZ], req[REQ_SZ];
    bool isTooLong=false;

    //check the number of arguments
    //hints setup
    memset(&hints, NULL, sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    //getting addrinfo
    if( (retVal = getaddrinfo(NULL, PORT_NB, &hints, &clientInfo) != 0) )
    {
        fprintf(stderr, "client: %s\n", gai_strerror(retVal));
        exit(EXIT_FAILURE);
    }
    //loop through all result
    for( p=clientInfo; p!=NULL; p=p->ai_next)
    {
        //open a socket
        if( (clientSock=
                socket(p->ai_family,p->ai_socktype, p->ai_protocol)
                )== -1)
        {
            perror("client: socket");
            continue;
        }
        //on success try to connect
        if( connect(clientSock, p->ai_addr, p->ai_addrlen) ==-1 )
        {
            close(clientSock);
            perror("client: connect");
            continue;
        }
        //on success connection - break the loop
        break;
    }
    //check if any connection exists
    if(p==NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(EXIT_FAILURE);
    }
    //printing info about connected server
    //inet_ntop(p->ai_family, (struct sockaddr *)p->ai_addr, buff,sizeof(buff));
    //fprintf(stdout, "client: connected to %s\n", buff);
    //freeing unneeded resources
    freeaddrinfo(clientInfo);
    memset(data,0,DATA_SZ);
    while( fgets(data, DATA_SZ, stdin) != NULL)
    {
        //row verification
        last = strlen(data) - 1;
        if( data[last] != '\n' )
        {
            // dropping chunks of too long row
            isTooLong = true;
            continue;
        }
        if(isTooLong)
        {
            //dropping last part of too long row
            isTooLong = false;
            fprintf(stderr, "Too long row dropped\n");
            continue;
        }
        data[last] = '\0';
        recvAskLine(clientSock, req, REQ_SZ);
        fprintf(stderr, "---\n");
        sendLine(clientSock, data, DATA_SZ);
        fprintf(stderr, "Client sent: %s\n", data); fflush(stderr);
    }
    recvAskLine(clientSock, data, DATA_SZ);
    sendLine(clientSock, FIN_STR, strlen(FIN_STR)+1);
    fprintf(stderr, "Client sent: %s\n", FIN_STR); fflush(stderr);
    close(clientSock);
    exit(EXIT_SUCCESS);
}

