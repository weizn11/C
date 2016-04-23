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
#include "../db/db.h"

extern unsigned long TotalConnections;
extern MEMPOOL_LIST *Mempool_IOData;
extern STACK_INFO Stack_IOData;

int clean_client_connection(IO_OPERATION_DATA_NODE *pIONode,int index)
{
    struct epoll_event event;
    STACK_DATA_TYPE IODataInfo;
    FRIEND_INFO_DATA friendInfoData;
    char *pFriendList=NULL,*pFriendNum=NULL;
    IO_OPERATION_DATA *pTmpIOData=NULL;

    memset(&IODataInfo,NULL,sizeof(STACK_DATA_TYPE));
    memset(&event,NULL,sizeof(struct epoll_event));
    memset(&friendInfoData,NULL,sizeof(friendInfoData));

    printf("Client \"%s\" logged off.TotalConnect:%d\n",inet_ntoa(pIONode->IOArray[index]->Address.sin_addr),\
           TotalConnections-1);

    strcat(friendInfoData.friendInfo.num,pIONode->IOArray[index]->ID);
    if(db_find_friend(&friendInfoData.friendInfo)==0)
    {
        friendInfoData.onlineStatus=0;
        pFriendList=db_search_friend_list(friendInfoData.friendInfo.num);
        if(pFriendList!=NULL)
        {
            pFriendNum=strtok(pFriendList,";");
            while(pFriendNum!=NULL)
            {
                if(search_IO_OPERATION_DATA_by_ID(pFriendNum,&pTmpIOData)==0)
                {
                    ex_send(pTmpIOData,(char *)&friendInfoData,sizeof(friendInfoData),6);
                }
                pFriendNum=strtok(NULL,";");
            }
            free(pFriendList);
        }
    }
    pthread_mutex_destroy(&pIONode->IOArray[index]->sendMutex);
    if(pIONode->IOArray[index]->recvBuffer!=NULL)
        mempool_free(Mempool_IOData,pIONode->IOArray[index]->recvBuffer);
    event.data.fd=pIONode->IOArray[index]->Socket;
    epoll_ctl(pIONode->epollfd,EPOLL_CTL_DEL,pIONode->IOArray[index]->Socket,&event);
    mempool_free(Mempool_IOData,pIONode->IOArray[index]);
    Mempool_IOData,pIONode->IOArray[index]=NULL;
    IODataInfo.index=index;
    IODataInfo.pIONode=pIONode;
    push_stack(&Stack_IOData,IODataInfo);
    TotalConnections--;

    return 0;
}










