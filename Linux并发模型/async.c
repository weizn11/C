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

unsigned long int TotalConnections=0;    //当前连接总数
unsigned long int ListenThreads=0;       //当前启动监听线程总数
pthread_rwlock_t LockIOData;            //IO操作读写锁
MEMPOOL_LIST *Mempool_IOData;              //IO_OPERATION_DATA结构内存池描述字
STACK_INFO Stack_IOData;                    //栈信息结构体
IO_OPERATION_DATA_NODE *IO_Operation_Data_Header=NULL;
MEMPOOL_LIST *mempoolListHeader=NULL;

int listen_client()
{
    int addrLength=0,index=0,createThread=0;
    pthread_t threadID;
    struct epoll_event event;
    SOCKET ClientSocket=INVALID_SOCKET,ListenSocket=INVALID_SOCKET;
    struct sockaddr_in ClientAddress,LocalAddress;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;
    STACK_DATA_TYPE IODataInfo;         //栈元素变量

    memset(&LocalAddress,NULL,sizeof(struct sockaddr_in));
    memset(&Stack_IOData,NULL,sizeof(STACK_INFO));

    LocalAddress.sin_family=AF_INET;
    LocalAddress.sin_port=htons(LISTEN_PORT);
    LocalAddress.sin_addr.s_addr=INADDR_ANY;
    pthread_rwlock_init(&LockIOData,NULL);
    init_stack(&Stack_IOData);              //栈在使用前需要进行初始化操作

    if((ListenSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
    {
        //create socket failed
        return -1;
    }

    if(bind(ListenSocket,(struct sockaddr *)&LocalAddress,sizeof(struct sockaddr_in))!=0)
    {
        //bind address failed
        return -2;
    }

    if(listen(ListenSocket,SOMAXCONN)!=0)
    {
        //listen socket failed
        return -3;
    }

    Mempool_IOData=create_mempool(sizeof(IO_OPERATION_DATA),ASYNC_MAX_WAIT_OBJECTS);  //创建IO_OPERATION_DATA结构内存池
    if(Mempool_IOData==NULL)
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
    IO_Operation_Data_Header->epollfd=epoll_create(ASYNC_MAX_WAIT_OBJECTS);         //创建epoll描述字
    //新建链表结点后将所有可用位置压入栈
    for(index=0; index<ASYNC_MAX_WAIT_OBJECTS; index++)
    {
        memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
        IODataInfo.pIONode=IO_Operation_Data_Header;
        IODataInfo.index=index;
        push_stack(&Stack_IOData,IODataInfo);
    }
    createThread=1;   //新建线程开关

    while(1)
    {
        addrLength=sizeof(struct sockaddr_in);
        ClientSocket=accept(ListenSocket,(struct sockaddr *)&ClientAddress,&addrLength);
        if(ClientSocket==INVALID_SOCKET) continue;

        pthread_rwlock_wrlock(&LockIOData);     //write lock
skip:
        //在栈中弹出一个可用的位置
        if(pop_stack(&Stack_IOData,&IODataInfo)==STACK_EMPTY)
        {
            //栈中已无可用位置，需要新建一个链表节点和新建一个线程
            for(IO_Operation_Node_Pointer=IO_Operation_Data_Header; IO_Operation_Node_Pointer->next!=NULL; \
                    IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next);
            IO_Operation_Node_Pointer->next=(IO_OPERATION_DATA_NODE *)malloc(sizeof(IO_OPERATION_DATA_NODE));
            if(IO_Operation_Node_Pointer->next==NULL)
            {
                //malloc error
                pthread_rwlock_unlock(&LockIOData); //unlock
                continue;
            }
            memset(IO_Operation_Node_Pointer->next,NULL,sizeof(IO_OPERATION_DATA_NODE));
            IO_Operation_Node_Pointer->next->epollfd=epoll_create(ASYNC_MAX_WAIT_OBJECTS);
            //新建的所有IO节点压入栈
            for(index=0; index<ASYNC_MAX_WAIT_OBJECTS; index++)
            {
                memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
                IODataInfo.pIONode=IO_Operation_Node_Pointer->next;
                IODataInfo.index=index;
                push_stack(&Stack_IOData,IODataInfo);
            }
            createThread=1;    //新建线程
            goto skip;
        }

        IO_Operation_Node_Pointer=IODataInfo.pIONode;
        index=IODataInfo.index;
        IO_Operation_Node_Pointer->IOArray[index]=(IO_OPERATION_DATA *)mempool_alloc(Mempool_IOData);   //从内存池中申请空间
        if(IO_Operation_Node_Pointer->IOArray[index]==NULL)
        {
            //malloc error
            pthread_rwlock_unlock(&LockIOData);     //unlock
            continue;
        }
        memset(IO_Operation_Node_Pointer->IOArray[index],NULL,sizeof(IO_OPERATION_DATA));
        //向存储有客户端相关信息的结构体中写入数据
        IO_Operation_Node_Pointer->IOArray[index]->Socket=ClientSocket;
        IO_Operation_Node_Pointer->IOArray[index]->Address=ClientAddress;
        IO_Operation_Node_Pointer->IOArray[index]->pIONode=IO_Operation_Node_Pointer;
        IO_Operation_Node_Pointer->IOArray[index]->posIndex=index;

        //向epoll中加入一个需要被监听的socket
        memset(&event,NULL,sizeof(struct epoll_event));
        event.events=EPOLLIN | EPOLLERR | EPOLLHUP;
        event.data.fd=ClientSocket;
        if(epoll_ctl(IO_Operation_Node_Pointer->epollfd,EPOLL_CTL_ADD,ClientSocket,&event)!=0)
        {
            //add socket failed
            pthread_rwlock_unlock(&LockIOData);
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
        pthread_rwlock_unlock(&LockIOData);     //unlock
    }

    return 0;
}






