#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>

#define SOCKET int
#define INVALID_SOCKET -1

#define TRAN_MAXIMUM_SIZE 2048
#define ASYNC_MAX_WAIT_OBJECTS 2048
#define SERVER_PORT_MAXIMUM 30
#define TRAN_MAX_MAP ASYNC_MAX_WAIT_OBJECTS/2-SERVER_PORT_MAXIMUM/2

typedef struct _Map_Info_
{
    char ip[30];
    int port[SERVER_PORT_MAXIMUM];
    struct _Map_Info_ *next;
} MAP_INFO;

MAP_INFO *Map_Info_Header=NULL;

MAP_INFO *add_map_info(MAP_INFO MapInfo)
{
    MAP_INFO *newNode=NULL,*pNode=NULL;

    newNode=(MAP_INFO *)malloc(sizeof(MAP_INFO));
    if(newNode==NULL)
    {
        printf("malloc error!\n");
        return NULL;
    }
    memset(newNode,NULL,sizeof(MAP_INFO));
    *newNode=MapInfo;

    if(Map_Info_Header==NULL)
        Map_Info_Header=newNode;
    else
    {
        for(pNode=Map_Info_Header; pNode->next!=NULL; pNode=pNode->next);
        pNode->next=newNode;
    }

    return newNode;
}

int map_port_thread(void *Parameter)
{
    int i,j,k,recvSize,activeCount,addrLength;
    int epollfd;
    char tranBuffer[TRAN_MAXIMUM_SIZE];
    MAP_INFO *pMapInfo=(MAP_INFO *)Parameter;
    struct epoll_event event;
    struct epoll_event eventList[ASYNC_MAX_WAIT_OBJECTS];
    SOCKET accSocket;
    SOCKET listenSocket[SERVER_PORT_MAXIMUM];
    SOCKET serverSocket[TRAN_MAX_MAP];
    SOCKET clientSocket[TRAN_MAX_MAP];
    struct sockaddr_in tmpAddress;
    struct sockaddr_in listenAddress[SERVER_PORT_MAXIMUM];
    struct sockaddr_in serverAddress[TRAN_MAX_MAP];
    struct sockaddr_in clientAddress[SERVER_PORT_MAXIMUM];

    pthread_detach(pthread_self());
    memset(listenSocket,NULL,sizeof(listenSocket));
    memset(serverAddress,NULL,sizeof(serverAddress));
    memset(clientAddress,NULL,sizeof(clientAddress));
    memset(listenAddress,NULL,sizeof(listenAddress));
    for(i=0; i<TRAN_MAX_MAP; i++)
    {
        serverSocket[i]=INVALID_SOCKET;
        clientSocket[i]=INVALID_SOCKET;
    }

    epollfd=epoll_create(ASYNC_MAX_WAIT_OBJECTS);
    for(i=0; pMapInfo->port[i]!=0; i++)
    {
        listenAddress[i].sin_family=AF_INET;
        listenAddress[i].sin_addr.s_addr=INADDR_ANY;
        listenAddress[i].sin_port=htons(pMapInfo->port[i]);

        clientAddress[i].sin_family=AF_INET;
        clientAddress[i].sin_addr.s_addr=inet_addr(pMapInfo->ip);
        clientAddress[i].sin_port=htons(pMapInfo->port[i]);

        listenSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(listenSocket[i]==INVALID_SOCKET)
        {
            printf("create socket failed!\n");
            continue;
        }

        if(bind(listenSocket[i],(struct sockaddr *)&listenAddress[i],sizeof(struct sockaddr_in))!=0)
        {
            printf("bind address failed!\n");
            continue;
        }
        if(listen(listenSocket[i],SOMAXCONN)!=0)
        {
            printf("listen socket failed!\n");
            continue;
        }

        memset(&event,NULL,sizeof(struct epoll_event));
        event.data.fd=listenSocket[i];
        event.events=EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,event.data.fd,&event);
    }

    while(1)
    {
        memset(eventList,NULL,sizeof(eventList));
        activeCount=epoll_wait(epollfd,eventList,ASYNC_MAX_WAIT_OBJECTS,100);
        if(activeCount<=0) continue;

        for(i=0; i<activeCount; i++)
        {
            for(k=0; k<SERVER_PORT_MAXIMUM; k++)
            {
                if(eventList[i].data.fd==listenSocket[k])
                {
                    if(eventList[i].events & EPOLLIN)
                    {
                        for(j=0; j<TRAN_MAX_MAP && serverSocket[j]!=INVALID_SOCKET; j++);
                        if(j==TRAN_MAX_MAP)
                        {
                            addrLength=sizeof(struct sockaddr_in);
                            accSocket=accept(listenSocket[k],(struct sockaddr *)&tmpAddress,&addrLength);
                            close(accSocket);
                            continue;
                        }
                        addrLength=sizeof(struct sockaddr_in);
                        serverSocket[j]=accSocket=accept(listenSocket[k],(struct sockaddr *)&serverAddress[j],&addrLength);
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=serverSocket[j];
                        event.events=EPOLLIN | EPOLLERR | EPOLLHUP;
                        epoll_ctl(epollfd,EPOLL_CTL_ADD,event.data.fd,&event);

                        clientSocket[j]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
                        if(clientSocket[j]==INVALID_SOCKET)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[j];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[j]);
                            serverSocket[j]=INVALID_SOCKET;
                            continue;
                        }
                        if(connect(clientSocket[j],(struct sockaddr *)&clientAddress[k],sizeof(struct sockaddr_in))!=0)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[j];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[j]);
                            serverSocket[j]=INVALID_SOCKET;
                            close(clientSocket[j]);
                            clientSocket[j]=INVALID_SOCKET;
                            continue;
                        }
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=clientSocket[j];
                        event.events=EPOLLIN | EPOLLERR | EPOLLHUP;
                        epoll_ctl(epollfd,EPOLL_CTL_ADD,event.data.fd,&event);
                    }
                }
            }

            for(k=0; k<TRAN_MAX_MAP; k++)
            {
                if(eventList[i].data.fd==serverSocket[k])
                {
                    if(eventList[i].events & EPOLLIN)
                    {
                        memset(tranBuffer,NULL,sizeof(tranBuffer));
                        recvSize=recv(eventList[i].data.fd,tranBuffer,sizeof(tranBuffer),0);
                        if(recvSize<=0)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=clientSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[k]);
                            serverSocket[k]=INVALID_SOCKET;
                            memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                            close(clientSocket[k]);
                            clientSocket[k]=INVALID_SOCKET;
                        }
                        if(send(clientSocket[k],tranBuffer,recvSize,0)<=0)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=clientSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[k]);
                            serverSocket[k]=INVALID_SOCKET;
                            memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                            close(clientSocket[k]);
                            clientSocket[k]=INVALID_SOCKET;
                        }
                    }
                    else
                    {
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=serverSocket[k];
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=clientSocket[k];
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                        close(serverSocket[k]);
                        serverSocket[k]=INVALID_SOCKET;
                        memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                        close(clientSocket[k]);
                        clientSocket[k]=INVALID_SOCKET;
                    }
                }
            }

            for(k=0; k<TRAN_MAX_MAP; k++)
            {
                if(eventList[i].data.fd==clientSocket[k])
                {
                    if(eventList[i].events & EPOLLIN)
                    {
                        memset(tranBuffer,NULL,sizeof(tranBuffer));
                        recvSize=recv(eventList[i].data.fd,tranBuffer,sizeof(tranBuffer),0);
                        if(recvSize<=0)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=clientSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[k]);
                            serverSocket[k]=INVALID_SOCKET;
                            memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                            close(clientSocket[k]);
                            clientSocket[k]=INVALID_SOCKET;
                        }
                        if(send(serverSocket[k],tranBuffer,recvSize,0)<=0)
                        {
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=serverSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            memset(&event,NULL,sizeof(struct epoll_event));
                            event.data.fd=clientSocket[k];
                            epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                            close(serverSocket[k]);
                            serverSocket[k]=INVALID_SOCKET;
                            memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                            close(clientSocket[k]);
                            clientSocket[k]=INVALID_SOCKET;
                        }
                    }
                    else
                    {
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=serverSocket[k];
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                        memset(&event,NULL,sizeof(struct epoll_event));
                        event.data.fd=clientSocket[k];
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,event.data.fd,&event);
                        close(serverSocket[k]);
                        serverSocket[k]=INVALID_SOCKET;
                        memset(&serverAddress[k],NULL,sizeof(struct sockaddr_in));
                        close(clientSocket[k]);
                        clientSocket[k]=INVALID_SOCKET;
                    }
                }
            }
        }
    }

    return 0;
}

int main()
{
    MAP_INFO MapInfo,*pMapInfo=NULL;
    int i;
    char strBuffer[1000],*pTmp=NULL;
    pthread_t threadData;

    printf("\t\t#############################################\n");
    printf("\t\t+                                           +\n");
    printf("\t\t+        PortMap for Linux (Ver 1.0)        +\n");
    printf("\t\t+                            -By:Wayne      +\n");
    printf("\t\t+                                           +\n");
    printf("\t\t#############################################\n");

    while(1)
    {
skip:
        printf("Please input the client's IP:\n");
        memset(strBuffer,NULL,sizeof(strBuffer));
        memset(&MapInfo,NULL,sizeof(MAP_INFO));
        fflush(stdin);
        gets(strBuffer);
        strcat(MapInfo.ip,strBuffer);
        printf("Please input map port(Whitespace separated):\n");
        memset(strBuffer,NULL,sizeof(strBuffer));
        fflush(stdin);
        gets(strBuffer);
        pTmp=strtok(strBuffer," ");
        i=0;
        do
        {
            MapInfo.port[i]=atoi(pTmp);
            pTmp=strtok(NULL," ");
            i++;
        }
        while(pTmp);
        pMapInfo=add_map_info(MapInfo);
        if(pMapInfo==NULL) continue;
        printf("mapping successfully!\n");
        if(pthread_create(&threadData,NULL,(void *)map_port_thread,(void *)pMapInfo)!=0)
        {
            printf("create new thread failed!\n");
        }
    }

    return 0;
}









