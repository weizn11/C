#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#include "p2p.h"
#include "../concurrent/async.h"

extern pthread_mutex_t MutexIOData;
extern IO_OPERATION_DATA_NODE *IO_Operation_Data_Header;

int check_empty(char *str,unsigned int size)
{
    int i;

    for(i=0; i<size; i++)
        if(str[i]!=NULL)
            return 0;

    return 1;
}

int p2p_create_conn_thread(void *Parameter)
{
    P2P_CREATE_CONN_THREAD_PARA *pThreadPara=(P2P_CREATE_CONN_THREAD_PARA *)Parameter;
    char recvBuffer[MAX_RECV_SIZE];
    P2P_CONN_INFO connInfo,*pConnInfo=(P2P_CONN_INFO *)&pThreadPara->connInfo;
    MAIN_PACKET mainPacket;
    int i,j;
    IO_OPERATION_DATA_NODE *pIONode=NULL;

    pthread_detach(pthread_self());

    pthread_mutex_lock(&MutexIOData);
    for(pIONode=IO_Operation_Data_Header; pIONode!=NULL; pIONode=pIONode->next)
        for(i=0; i<ASYNC_MAX_WAIT_OBJECTS; i++)
            if(pIONode->IOArray[i]!=NULL)
                if(!strcmp(pIONode->IOArray[i]->ID,pConnInfo->toID))
                {
                    memset(&connInfo,NULL,sizeof(P2P_CONN_INFO));
                    for(j=0; j<10; j++)
                    {
                        pthread_mutex_unlock(&MutexIOData);
                        usleep(500000);
                        pthread_mutex_lock(&MutexIOData);
                        if(pThreadPara->pIONode->IOArray[pThreadPara->index]!=NULL)
                            if(check_empty((char *)&pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr,\
                                           sizeof(struct sockaddr_in))==0)
                            {
                                connInfo.toAddr=pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr;
                                strcat(connInfo.toID,pConnInfo->fromID);
                                strcat(connInfo.fromID,pConnInfo->toID);
                                connInfo.flag=pConnInfo->flag;
                                printf("%s:%d\n",inet_ntoa(pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr.sin_addr),\
                                       ntohs(pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr.sin_port));
                                memset(&pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr,NULL,\
                                       sizeof(struct sockaddr_in));
                                break;
                            }
                    }
                    if(j>=10)
                    {
                        //timeout
                        printf("wait client1 addr timeout\n");
                        pthread_mutex_unlock(&MutexIOData);
                        free(pThreadPara);
                        return -1;
                    }

                    pthread_mutex_lock(&pIONode->IOArray[i]->sendMutex);
                    memset(&mainPacket,NULL,sizeof(MAIN_PACKET));
                    memcpy(mainPacket.data,(char *)&connInfo,sizeof(P2P_CONN_INFO));
                    mainPacket.proto=1;
                    if(send(pIONode->IOArray[i]->Socket,(char *)&mainPacket,sizeof(MAIN_PACKET),0)<=0)
                    {
                        pthread_mutex_unlock(&pIONode->IOArray[i]->sendMutex);
                        pthread_mutex_unlock(&MutexIOData);
                        return -2;
                    }
                    printf("Send p2p request to client2\n");
                    pthread_mutex_unlock(&pIONode->IOArray[i]->sendMutex);

                    memset(&pThreadPara->pIONode->IOArray[pThreadPara->index]->p2pAddr,NULL,sizeof(struct sockaddr_in));


                    memset(&connInfo,NULL,sizeof(P2P_CONN_INFO));
                    for(j=0; j<10; j++)
                    {
                        pthread_mutex_unlock(&MutexIOData);
                        usleep(500000);
                        pthread_mutex_lock(&MutexIOData);
                        if(pIONode->IOArray[i]!=NULL)
                            if(check_empty((char *)&pIONode->IOArray[i]->p2pAddr,\
                                           sizeof(struct sockaddr_in))==0)
                            {
                                connInfo.toAddr=pIONode->IOArray[i]->p2pAddr;
                                strcat(connInfo.toID,pConnInfo->toID);
                                strcat(connInfo.fromID,pConnInfo->fromID);
                                connInfo.flag=0;
                                printf("%s:%d\n",inet_ntoa(pIONode->IOArray[i]->p2pAddr.sin_addr),\
                                       ntohs(pIONode->IOArray[i]->p2pAddr.sin_port));
                                memset(&pIONode->IOArray[i]->p2pAddr,NULL,\
                                       sizeof(struct sockaddr_in));
                                break;
                            }
                    }
                    if(j>=10)
                    {
                        //timeout
                        printf("wait client2 addr timeout\n");
                        pthread_mutex_unlock(&MutexIOData);
                        free(pThreadPara);
                        return -1;
                    }

                    pthread_mutex_lock(&pThreadPara->pIONode->IOArray[pThreadPara->index]->sendMutex);
                    memset(&mainPacket,NULL,sizeof(MAIN_PACKET));
                    memcpy(mainPacket.data,(char *)&connInfo,sizeof(P2P_CONN_INFO));
                    mainPacket.proto=1;
                    if(send(pThreadPara->pIONode->IOArray[pThreadPara->index]->Socket,\
                            (char *)&mainPacket,sizeof(MAIN_PACKET),0)<=0)
                    {
                        pthread_mutex_unlock(&pThreadPara->pIONode->IOArray[pThreadPara->index]->sendMutex);
                        pthread_mutex_unlock(&MutexIOData);
                        return -2;
                    }
                    printf("Send p2p request to client1\n");
                    pthread_mutex_unlock(&pThreadPara->pIONode->IOArray[pThreadPara->index]->sendMutex);

                    memset(&pIONode->IOArray[i]->p2pAddr,NULL,sizeof(struct sockaddr_in));

                }
    pthread_mutex_unlock(&MutexIOData);
    free(pThreadPara);

    return 0;
}

int p2p_listen_server()
{
    int i,recvSize,addrLength;
    SOCKET listenSocket;
    struct sockaddr_in listenAddress,clientAddress;
    P2P_TRAN_INFO_DATA tranData;
    char recvBuffer[P2P_DATA_MAX_SIZE];
    P2P_CONN_INFO *pConnInfo=NULL;
    IO_OPERATION_DATA_NODE *pIONode=NULL;
    pthread_t threadID;
    P2P_CREATE_CONN_THREAD_PARA *pThreadPara=NULL;

    memset(&listenAddress,NULL,sizeof(struct sockaddr_in));
    memset(&tranData,NULL,sizeof(P2P_TRAN_INFO_DATA));

    listenAddress.sin_family=AF_INET;
    listenAddress.sin_addr.s_addr=INADDR_ANY;
    listenAddress.sin_port=htons(P2P_UDP_LISTEN_PORT);
    pConnInfo=(P2P_CONN_INFO *)recvBuffer;

    listenSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(listenSocket==INVALID_SOCKET)
    {
        printf("create P2P socket failed!\n");
        return -1;
    }

    if(bind(listenSocket,(struct sockaddr *)&listenAddress,sizeof(struct sockaddr_in))!=0)
    {
        printf("bind P2P address failed!\n");
        return -2;
    }

    while(1)
    {
        memset(recvBuffer,NULL,sizeof(recvBuffer));
        memset(&clientAddress,NULL,sizeof(struct sockaddr_in));
        printf("P2P Server listening...\n");
        addrLength=sizeof(struct sockaddr_in);
        recvSize=recvfrom(listenSocket,recvBuffer,sizeof(recvBuffer),0,&clientAddress,&addrLength);
        printf("P2P Client from %s:%d,FromID:%s,ToID:%s\n",inet_ntoa(clientAddress.sin_addr),ntohs(clientAddress.sin_port),\
               pConnInfo->fromID,pConnInfo->toID);
        if(recvSize!=sizeof(P2P_CONN_INFO))
            continue;

        pthread_mutex_lock(&MutexIOData);
        for(pIONode=IO_Operation_Data_Header; pIONode!=NULL; pIONode=pIONode->next)
            for(i=0; i<ASYNC_MAX_WAIT_OBJECTS; i++)
            {
                if(pIONode->IOArray[i]!=NULL)
                {
                    if(!strcmp(pIONode->IOArray[i]->ID,pConnInfo->fromID))
                    {
                        //printf("find fromID:%s,index:%d\n",pIONode->IOArray[i]->ID,i);
                        pIONode->IOArray[i]->p2pAddr=clientAddress;
                        //printf("Write to list P2P client '%s:%d' online,from:%s,to:%s\n",\
                               inet_ntoa(pIONode->IOArray[2046]->p2pAddr.sin_addr),\
                               ntohs(pIONode->IOArray[2046]->p2pAddr.sin_port),\
                               pConnInfo->fromID,pConnInfo->toID);
                        goto skip;
                    }
                }
            }
skip:
        pthread_mutex_unlock(&MutexIOData);
    }

    return 0;
}

