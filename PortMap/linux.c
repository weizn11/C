#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 20480
#define SOCKET int
#define INVALID_SOCKET -1

SOCKET create_socket(char *ip,int port)
{
    SOCKET soc=INVALID_SOCKET;
    struct sockaddr_in addr;

    memset(&addr,NULL,sizeof(struct sockaddr_in));

    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);

    if((soc=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
        return INVALID_SOCKET;

    if(connect(soc,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))!=0)
    {
        close(soc);
        return INVALID_SOCKET;
    }

    return soc;
}

int transfer_data(SOCKET soc1,SOCKET soc2)
{
    int maxfd;
    fd_set readfd;
    struct timeval timeout;
    char buffer[MAX_SIZE+1];
    unsigned int RecvSize;

    memset(&timeout,NULL,sizeof(struct timeval));
    timeout.tv_usec=100;

    while(1)
    {
        FD_ZERO(&readfd);
        FD_SET(soc1,&readfd);
        FD_SET(soc2,&readfd);
        maxfd=(soc1>soc2?soc1+1:soc2+1);
        if(select(maxfd,&readfd,NULL,NULL,&timeout)<1)
            continue;
        if(FD_ISSET(soc1,&readfd))
        {
            memset(buffer,NULL,sizeof(buffer));
            if((RecvSize=recv(soc1,buffer,MAX_SIZE,0))<=0)
                return -1;
            if(send(soc2,buffer,RecvSize,0)<=0)
                return -2;
            printf("RemotePort -----> TargetPort   PacketSize:%d\n",RecvSize);
        }
        if(FD_ISSET(soc2,&readfd))
        {
            memset(buffer,NULL,sizeof(buffer));
            if((RecvSize=recv(soc2,buffer,MAX_SIZE,0))<=0)
                return -2;
            if(send(soc1,buffer,RecvSize,0)<=0)
                return -1;
            printf("RemotePort <----- TargetPort   PacketSize:%d\n",RecvSize);
        }
    }

    return 0;
}

int main(int argc,char *argv[])
{
    int RemotePort,TargetPort;
    SOCKET soc1,soc2;

    if(argc!=6 || strcmp(argv[1],"-remote")!=0)
    {
        printf("Usage:./XXX -remote RemoteIP RemotePort TargetIP TargetPort\n");
        return -1;
    }

    RemotePort=atoi(argv[3]);
    TargetPort=atoi(argv[5]);
    printf("Connect to %s:%d\n",argv[2],RemotePort);
    if((soc1=create_socket(argv[2],RemotePort))==INVALID_SOCKET)
    {
        printf("Can't connect to %s:%d\n",argv[2],RemotePort);
        return -1;
    }
    printf("Connect to %s:%d successfully!\n",argv[2],RemotePort);

    printf("Connect to %s:%d\n",argv[4],TargetPort);
    if((soc2=create_socket(argv[4],TargetPort))==INVALID_SOCKET)
    {
        printf("Can't connect to %s:%d\n",argv[4],TargetPort);
        return -1;
    }
    printf("Connect to %s:%d successfully!\n",argv[4],TargetPort);

    if(transfer_data(soc1,soc2)==-1)
    {
        printf("Remote host disconnect\n");
    }
    else
    {
        printf("Target host disconnect\n");
    }

    return 0;
}




