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

#include "async.h"
#include "recv.h"
#include "mempool.h"
#include "stack.h"

unsigned long int TotalConnections=0;
unsigned long int ListenThreads=0;
pthread_mutex_t MutexIOData;
MEMPOOL_LIST *Mempool_IOData;
MEMPOOL_LIST *Mempool_RecvBuffer;
STACK_INFO Stack_IOData;
IO_OPERATION_DATA_NODE *IO_Operation_Data_Header=NULL;

int listen_client()
{
    int addrLength=0,index=0,createThread=0;
    pthread_t threadID;
    struct epoll_event event;
    SOCKET ClientSocket=INVALID_SOCKET,ListenSocket=INVALID_SOCKET;
    struct sockaddr_in ClientAddress,LocalAddress;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;
    STACK_DATA_TYPE IODataInfo;

    memset(&LocalAddress,NULL,sizeof(struct sockaddr_in));
    memset(&Stack_IOData,NULL,sizeof(STACK_INFO));

    LocalAddress.sin_family=AF_INET;
    LocalAddress.sin_port=htons(LISTEN_PORT);
    LocalAddress.sin_addr.s_addr=INADDR_ANY;
    pthread_mutex_init(&MutexIOData,NULL);
    init_stack(&Stack_IOData);

    if((ListenSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
    {
        //create socket failed
        printf("create socket failed\n");
        return -1;
    }

    if(bind(ListenSocket,(struct sockaddr *)&LocalAddress,sizeof(struct sockaddr_in))!=0)
    {
        //bind address failed
        printf("bind address failed\n");
        return -2;
    }

    if(listen(ListenSocket,SOMAXCONN)!=0)
    {
        //listen socket failed
        printf("listen socket failed\n");
        return -3;
    }

    Mempool_IOData=create_mempool(sizeof(IO_OPERATION_DATA),IODATA_MEMPOOL_MAXIMUM_CELL);
    Mempool_RecvBuffer=create_mempool(sizeof(MAIN_PACKET),RECV_MEMPOOL_MAXIMUM_CELL);
    if(Mempool_IOData==NULL || Mempool_RecvBuffer==NULL)
    {
        printf("Create mempool failed.\n");
        return -4;
    }

    IO_Operation_Data_Header=(IO_OPERATION_DATA_NODE *)malloc(sizeof(IO_OPERATION_DATA_NODE));
    if(IO_Operation_Data_Header==NULL)
    {
        //malloc error
        return -5;
    }
    memset(IO_Operation_Data_Header,NULL,sizeof(IO_OPERATION_DATA_NODE));
    IO_Operation_Data_Header->epollfd=epoll_create(ASYNC_MAX_WAIT_OBJECTS);
    for(index=0; index<ASYNC_MAX_WAIT_OBJECTS; index++)
    {
        memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
        IODataInfo.pIONode=IO_Operation_Data_Header;
        IODataInfo.index=index;
        push_stack(&Stack_IOData,IODataInfo);
    }
    createThread=1;

    printf("TCP Server Listening...\n");

    while(1)
    {
        addrLength=sizeof(struct sockaddr_in);
        ClientSocket=accept(ListenSocket,(struct sockaddr *)&ClientAddress,&addrLength);
        if(ClientSocket==INVALID_SOCKET) continue;

        pthread_mutex_lock(&MutexIOData);
skip:
        if(pop_stack(&Stack_IOData,&IODataInfo)==STACK_EMPTY)
        {
            //all the thread's task is full
            for(IO_Operation_Node_Pointer=IO_Operation_Data_Header; IO_Operation_Node_Pointer->next!=NULL; \
                    IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next);
            IO_Operation_Node_Pointer->next=(IO_OPERATION_DATA_NODE *)malloc(sizeof(IO_OPERATION_DATA_NODE));
            if(IO_Operation_Node_Pointer->next==NULL)
            {
                //malloc error
                pthread_mutex_unlock(&MutexIOData);
                continue;
            }
            memset(IO_Operation_Node_Pointer->next,NULL,sizeof(IO_OPERATION_DATA_NODE));
            IO_Operation_Node_Pointer->next->epollfd=epoll_create(ASYNC_MAX_WAIT_OBJECTS);
            for(index=0; index<ASYNC_MAX_WAIT_OBJECTS; index++)
            {
                memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
                IODataInfo.pIONode=IO_Operation_Node_Pointer->next;
                IODataInfo.index=index;
                push_stack(&Stack_IOData,IODataInfo);
            }
            createThread=1;
            goto skip;
        }

        IO_Operation_Node_Pointer=IODataInfo.pIONode;
        index=IODataInfo.index;
        IO_Operation_Node_Pointer->IOArray[index]=(IO_OPERATION_DATA *)mempool_alloc(Mempool_IOData);
        if(IO_Operation_Node_Pointer->IOArray[index]==NULL)
        {
            //malloc error
            pthread_mutex_unlock(&MutexIOData);
            continue;
        }
        memset(IO_Operation_Node_Pointer->IOArray[index],NULL,sizeof(IO_OPERATION_DATA));
        IO_Operation_Node_Pointer->IOArray[index]->Socket=ClientSocket;
        IO_Operation_Node_Pointer->IOArray[index]->Address=ClientAddress;
        IO_Operation_Node_Pointer->IOArray[index]->index=index;
        IO_Operation_Node_Pointer->IOArray[index]->pIONode=IO_Operation_Node_Pointer;
        pthread_mutex_init(&IO_Operation_Node_Pointer->IOArray[index]->sendMutex,NULL);

        if(TotalConnections==0)
            sprintf(IO_Operation_Node_Pointer->IOArray[index]->ID,"%d",2);
        else
            sprintf(IO_Operation_Node_Pointer->IOArray[index]->ID,"%d",1);
        printf("current ID:%s\n",IO_Operation_Node_Pointer->IOArray[index]->ID);

        memset(&event,NULL,sizeof(struct epoll_event));
        event.events=EPOLLIN | EPOLLERR | EPOLLHUP;
        event.data.fd=ClientSocket;
        if(epoll_ctl(IO_Operation_Node_Pointer->epollfd,EPOLL_CTL_ADD,ClientSocket,&event)!=0)
        {
            //add socket failed
            pthread_mutex_unlock(&MutexIOData);
            continue;
        }

        TotalConnections++;
        if(createThread)
        {
            //create a new listen thread
            ListenThreads++;
            createThread=0;
            if(pthread_create(&threadID,NULL,(void *)listen_socket_from_client,(void *)IO_Operation_Node_Pointer)!=0)
            {
                printf("create thread failed.\n");
            }
        }

        printf("Client \"%s\" online.TotalConnect:%d\n",inet_ntoa(ClientAddress.sin_addr),TotalConnections);
        pthread_mutex_unlock(&MutexIOData);
    }

    return 0;
}






