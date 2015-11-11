#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


int sendLine(int dstSock, char *buff, int buffSz)
{
    int bytesCnt;
    if( (bytesCnt=send(dstSock, buff, buffSz, 0)) < 0)
    {
            perror("server send");
            exit(EXIT_FAILURE);
    }
    return bytesCnt;
}

int recvLine(const int srcSock, char *buff, int buffSz)
{
    /* The line cannot be longer than buffSz.
     * msgLen only tells how many bytes have been received, not how many are
     * available, so it's just check against last symbol is \0
     * The buffer is zeroized at the beginning */
    int msgLen, bytesCnt;
    memset(buff, '0',  buffSz);
    for(bytesCnt=0; bytesCnt<buffSz;)
    {
        msgLen=recv(srcSock, buff+bytesCnt, buffSz-bytesCnt, 0);
        if(msgLen==0)  //client connection's closed
            break;
        if(msgLen<0)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        bytesCnt += msgLen;
        if (buff[bytesCnt-1] == '\0')
            break;
    }
    return bytesCnt;
}

