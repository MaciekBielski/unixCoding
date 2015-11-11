/* show an IP for a host given on a command-line
 *
 * getaddrinfo finds a suitable endpoint for socket connection 
 * */
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>

int main(int argc, char** argv)
{
  /* hints - criteria for a structure created and binded with res */
  struct addrinfo hints, *res, *ptr;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  if(argc !=2)
  {
    fprintf(stdout,"usage %s <ip>\n", argv[0]);
    return 1;
  }
  /* zeroize all fields of input addrinfo, set some of them*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; //node has to be null
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  //hints.ai_flags = AI_PASSIVE; for passive sockets
  /* 1.
   * perform lookup and set res to list of addrinfo structures
   * ALLOCATION is done here
   */
  if( (status=getaddrinfo(argv[1],"80",&hints, &res))!=0 )
  {
    fprintf(stderr, "errro: %s\n", gai_strerror(status));
    exit(2);
  }
  fprintf(stdout,"IP addresses for %s:\n",argv[1]);
  /* 2.
   * res points to ALLOCATED linked list of returned addrinfos
   */
  for(ptr=res; ptr!=NULL; ptr=ptr->ai_next)
  {
    void *addr=NULL;
    char *ipver=NULL;
    if(ptr->ai_family==AF_INET)
    {
      /* get IPv4 address as a binary value */
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;  
      addr = &(ipv4->sin_addr);
      ipver="IPV4";
    }
    else if(ptr->ai_family==AF_INET6)
    {
      /* get IPv6 address as a binary value */
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ptr->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver="IPV6";
    }
    /* 3.
     * translate binary value of an address to printable string
     */
    inet_ntop(ptr->ai_family, addr, ipstr, sizeof ipstr);
    fprintf(stdout,"\t%s: %s\n",ipver,ipstr);
  }
  /* 4.
   * free the allocated linked list
   */
  freeaddrinfo(res);
}

