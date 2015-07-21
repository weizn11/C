#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>

#define MAX_SIZE 20480

int init_socket()
{
    WSADATA wsa;

    memset(&wsa,NULL,sizeof(WSADATA));

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
        return -1;

    return 0;
}

SOCKET create_socket(int port)
{
    SOCKET soc=INVALID_SOCKET,soc2=INVALID_SOCKET;
    struct sockaddr_in addr;
    unsigned int AddrLen=0;

    memset(&addr,NULL,sizeof(struct sockaddr_in));

    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);

    if((soc=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
        return INVALID_SOCKET;

    if(bind(soc,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))!=0)
    {
        closesocket(soc);
        return INVALID_SOCKET;
    }

    if(listen(soc,SOMAXCONN)!=0)
    {
        closesocket(soc);
        return INVALID_SOCKET;
    }

    AddrLen=sizeof(struct sockaddr_in);
    soc2=accept(soc,(struct sockaddr *)&addr,&AddrLen);
    if(soc2==INVALID_SOCKET)
    {
        closesocket(soc);
        return INVALID_SOCKET;
    }

    return soc2;
}

int transfer_data(SOCKET soc1,SOCKET soc2)
{
    unsigned int RecvSize;
    fd_set readfd,writefd;
    struct timeval timeout;
    char buffer[MAX_SIZE+1];

    memset(&timeout,NULL,sizeof(struct timeval));
    timeout.tv_usec=100;
    while(1)
    {
        FD_ZERO(&readfd);
        //FD_ZERO(&writefd);
        FD_SET(soc1,&readfd);
        //FD_SET(soc1,&writefd);
        FD_SET(soc2,&readfd);
        //FD_SET(soc2,&writefd);
        if(select(-1,&readfd,NULL,NULL,&timeout)<1)
            continue;

        if(FD_ISSET(soc1,&readfd))
        {
            memset(buffer,NULL,sizeof(buffer));
            if((RecvSize=recv(soc1,buffer,MAX_SIZE,0))<0)
                return -1;
            if(send(soc2,buffer,RecvSize,0)<=0)
                return -2;
            printf("Port1 -------> Port2   PacketSize:%d\n",RecvSize);
        }
        if(FD_ISSET(soc2,&readfd))
        {
            memset(buffer,NULL,sizeof(buffer));
            if((RecvSize=recv(soc2,buffer,MAX_SIZE,0))<0)
                return -2;
            if(send(soc1,buffer,RecvSize,0)<=0)
                return -1;
            printf("Port1 <------- Port2   PacketSize:%d\n",RecvSize);
        }
    }

    return 0;
}

int main(int argc,char *argv[])
{
    unsigned int ListenPort,ConnectPort;
    SOCKET ListenPortSocket,ConnectPortSocket;
    system("color a");
    if(argc!=4 || strcmp("-listen",argv[1])!=0)
    {
        printf("Usage:PortMap.exe -listen ListenPort ConnectPort\n");
        return -1;
    }
    if(init_socket()!=0)
    {
        printf("Init socket error!\n");
        return -1;
    }
    ListenPort=atoi(argv[2]);
    ConnectPort=atoi(argv[3]);
    printf("Listening port on %d...\n",ListenPort);
    ListenPortSocket=create_socket(ListenPort);
    if(ListenPortSocket==INVALID_SOCKET)
    {
        printf("Create listen socket error\n");
        return -1;
    }
    printf("Listening port on %d...\n",ConnectPort);
    ConnectPortSocket=create_socket(ConnectPort);
    if(ConnectPortSocket==INVALID_SOCKET)
    {
        printf("Create connect socket error\n");
        return -1;
    }
    printf("Buid connect successfully!\n");
    transfer_data(ListenPortSocket,ConnectPortSocket);
    printf("Connection has been disconnected!\n");

    return 0;
}
