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
#include <errno.h>

#include "recv.h"
#include "async.h"
#include "cleanup.h"
#include "mempool.h"
#include "../p2p/p2p.h"

extern unsigned long int TotalConnections;
extern unsigned long int ListenThreads;
extern pthread_mutex_t MutexIOData;
extern IO_OPERATION_DATA_NODE *IO_Operation_Data_Header;
extern MEMPOOL_LIST *Mempool_RecvBuffer;

int recv_data_from_client(IO_OPERATION_DATA *pIOData)
{
    int recvSize;
    char *recvPointer=NULL;
    MAIN_PACKET *pPacket=NULL;
    P2P_CONN_INFO *pConnInfo=NULL;
    P2P_CREATE_CONN_THREAD_PARA *pP2PCreateConnThreadPara=NULL;
    pthread_t threadID;

    memset(&threadID,NULL,sizeof(pthread_t));

    if(pIOData->recvBuffer==NULL)
    {
        pIOData->recvSize=0;
        pIOData->recvBuffer=(char *)mempool_alloc(Mempool_RecvBuffer);
        if(pIOData->recvBuffer==NULL) return 0;
        memset(pIOData->recvBuffer,NULL,sizeof(MAIN_PACKET));
        recvPointer=pIOData->recvBuffer;
    }
    recvPointer=pIOData->recvBuffer+pIOData->recvSize;

    recvSize=recv(pIOData->Socket,recvPointer,sizeof(MAIN_PACKET)-pIOData->recvSize,0);
    if(recvSize<=0)
    {
        //socket disable
        printf("recv failed!Error code:%d\n",errno);
        return -2;
    }
    if(recvSize+pIOData->recvSize>sizeof(MAIN_PACKET))
    {
        mempool_free(Mempool_RecvBuffer,pIOData->recvBuffer);
        pIOData->recvBuffer=NULL;
        pIOData->recvSize=0;
        return -3;
    }
    pIOData->recvSize+=recvSize;

    if(pIOData->recvSize==sizeof(MAIN_PACKET))
    {
        pPacket=(MAIN_PACKET *)pIOData->recvBuffer;
        switch(pPacket->proto)
        {
        case 1:
            printf("recv p2p conn request\n");
            pConnInfo=(P2P_CONN_INFO *)pPacket->data;
            pP2PCreateConnThreadPara=(P2P_CREATE_CONN_THREAD_PARA *)malloc(sizeof(P2P_CREATE_CONN_THREAD_PARA));
            if(pP2PCreateConnThreadPara==NULL) break;
            memset(pP2PCreateConnThreadPara,NULL,sizeof(P2P_CREATE_CONN_THREAD_PARA));
            pP2PCreateConnThreadPara->connInfo=*pConnInfo;
            pP2PCreateConnThreadPara->index=pIOData->index;
            pP2PCreateConnThreadPara->pIONode=pIOData->pIONode;
            if(pthread_create(&threadID,NULL,p2p_create_conn_thread,(void *)pP2PCreateConnThreadPara)!=0)
            {
                //create thread failed.
            }
            break;
        default:
            break;
        }
        mempool_free(Mempool_RecvBuffer,pIOData->recvBuffer);
        pIOData->recvBuffer=NULL;
        pIOData->recvSize=0;
    }

    return recvSize;
}

int listen_socket_from_client(void *Parameter)
{
    int activeCount;
    int i,j;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;
    struct epoll_event eventList[ASYNC_MAX_WAIT_OBJECTS];

    printf("Create a new thread\n");

    pthread_detach(pthread_self());
    IO_Operation_Node_Pointer=(IO_OPERATION_DATA_NODE *)Parameter;

    while(1)
    {
        activeCount=epoll_wait(IO_Operation_Node_Pointer->epollfd,eventList,ASYNC_MAX_WAIT_OBJECTS,SOCKET_TIMEOUT);
        if(activeCount<=0) continue;

        for(j=0; j<ASYNC_MAX_WAIT_OBJECTS; j++)
        {
            if(IO_Operation_Node_Pointer->IOArray[j]==NULL) continue;
            for(i=0; i<activeCount; i++)
            {
                if(IO_Operation_Node_Pointer->IOArray[j]->Socket==eventList[i].data.fd)
                {
                    if(eventList[i].events & EPOLLIN)
                    {
                        if(recv_data_from_client(IO_Operation_Node_Pointer->IOArray[j])<=0)
                        {
                            //clean sockset;
                            pthread_mutex_lock(&MutexIOData);
                            clean_client_connection(IO_Operation_Node_Pointer,j);
                            pthread_mutex_unlock(&MutexIOData);
                        }
                    }
                    else
                    {
                        //client disconnect
                        pthread_mutex_lock(&MutexIOData);
                        clean_client_connection(IO_Operation_Node_Pointer,j);
                        pthread_mutex_unlock(&MutexIOData);
                    }
                    break;
                }
            }
        }
    }

    return 0;
}










