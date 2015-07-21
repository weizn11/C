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

#include "cleanup.h"
#include "async.h"
#include "mempool.h"
#include "stack.h"

extern unsigned long TotalConnections;
extern MEMPOOL_LIST *Mempool_IOData;
extern STACK_INFO Stack_IOData;

int clean_client_connection(IO_OPERATION_DATA_NODE *pIONode,int index)
{
    //当客户端断开链结后清理内存中的相关数据
    struct epoll_event event;
    STACK_DATA_TYPE IODataInfo;

    memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
    memset(&event,NULL,sizeof(struct epoll_event));

    printf("Client \"%s\" logged off.TotalConnect:%d\n",inet_ntoa(pIONode->IOArray[index]->Address.sin_addr),\
           TotalConnections-1);

    //还有接收过的数据存储在内存中，释放这块内存
    if(pIONode->IOArray[index]->recvBuffer!=NULL)
        free(pIONode->IOArray[index]->recvBuffer);
    //在epoll中删除相关的socket
    event.data.fd=pIONode->IOArray[index]->Socket;
    epoll_ctl(pIONode->epollfd,EPOLL_CTL_DEL,pIONode->IOArray[index]->Socket,&event);
    mempool_free(Mempool_IOData,pIONode->IOArray[index]);       //归还内存池
    Mempool_IOData,pIONode->IOArray[index]=NULL;
    IODataInfo.index=index;
    IODataInfo.pIONode=pIONode;
    push_stack(&Stack_IOData,IODataInfo);       //将此处可用的位置压入到栈中
    TotalConnections--;

    return 0;
}










