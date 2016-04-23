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
#include <dirent.h>

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

int ex_send(IO_OPERATION_DATA *pIOData,char *sendBuffer,unsigned int bufferSize,int proto)
{
    int sendSize;
    MAIN_PACKET Packet;

    memset(&Packet,NULL,sizeof(MAIN_PACKET));

    Packet.proto=proto;
    memcpy(Packet.data,sendBuffer,bufferSize);

    pthread_mutex_lock(&pIOData->sendMutex);
    sendSize=send(pIOData->Socket,(char *)&Packet,sizeof(MAIN_PACKET),0);
    pthread_mutex_unlock(&pIOData->sendMutex);

    return sendSize;
}

int search_IO_OPERATION_DATA_by_ID(char *ID,IO_OPERATION_DATA **ppIOData)
{
    int i;
    IO_OPERATION_DATA_NODE *pIONode=NULL;

    for(pIONode=IO_Operation_Data_Header; pIONode!=NULL; pIONode=pIONode->next)
    {
        for(i=0; i<ASYNC_MAX_WAIT_OBJECTS; i++)
        {
            if(pIONode->IOArray[i]!=NULL)
            {
                if(!strcmp(ID,pIONode->IOArray[i]->ID))
                {
                    if(ppIOData!=NULL)
                        *ppIOData=pIONode->IOArray[i];
                    return 0;
                }
            }
        }
    }
    if(ppIOData!=NULL)
        *ppIOData=NULL;

    return -1;
}

int broadcast_data_to_friend(IO_OPERATION_DATA *pIOData,char *pData,int dataSize,int proto)
{
    char *pFriendList=NULL,*pFriendNum=NULL;
    IO_OPERATION_DATA *pTmpIOData=NULL;

    pFriendList=db_search_friend_list(pIOData->ID);
    if(pFriendList==NULL)
        return -1;
    pFriendNum=strtok(pFriendList,";");
    while(pFriendNum!=NULL)
    {
        printf("find friend '%s'\n",pFriendNum);
        if(search_IO_OPERATION_DATA_by_ID(pFriendNum,&pTmpIOData)==0)
        {
            ex_send(pTmpIOData,(char *)pData,dataSize,proto);
        }
        pFriendNum=strtok(NULL,";");
    }
    free(pFriendList);

    return 0;
}

int send_icon_to_client(char *num,IO_OPERATION_DATA *pIOData)
{
    int i=1;
    FILE *pFile=NULL;
    ICON_TRAN_DATA iconData;
    char iconPath[255];

    memset(iconPath,NULL,sizeof(iconPath));

    sprintf(iconPath,"images/icon/%s.jpg",num);
    pFile=fopen(iconPath,"rb");
    if(pFile==NULL)
    {
        printf("open '%s' failed.\n",iconPath);
        return -1;
    }
    do
    {
        memset(&iconData,NULL,sizeof(iconData));
        iconData.dataSize=fread(iconData.data,sizeof(char),sizeof(iconData.data),pFile);
        strcat(iconData.num,num);
        if(feof(pFile))
            iconData.flag=0;
        else
            iconData.flag=1;
        if(i)
        {
            iconData.flushFlag=1;
            i=0;
        }
        ex_send(pIOData,(char *)&iconData,sizeof(iconData),8);
    }while(iconData.flag);
    fclose(pFile);

    return 0;
}

int broadcast_icon_to_friend(IO_OPERATION_DATA *pIOData)
{
    char *pFriendList=NULL,*pFriendNum=NULL;
    IO_OPERATION_DATA *pTmpIOData=NULL;

    send_icon_to_client(pIOData->ID,pIOData);
    pFriendList=db_search_friend_list(pIOData->ID);
    if(pFriendList==NULL)
        return -1;
    pFriendNum=strtok(pFriendList,";");
    while(pFriendNum!=NULL)
    {
        printf("find friend '%s'\n",pFriendNum);
        if(search_IO_OPERATION_DATA_by_ID(pFriendNum,&pTmpIOData)==0)
        {
            send_icon_to_client(pIOData->ID,pTmpIOData);
        }
        pFriendNum=strtok(NULL,";");
    }
    free(pFriendList);

    return 0;
}

int send_friend_icon_to_me(IO_OPERATION_DATA *pIOData)
{
    char *pFriendList=NULL,*pFriendNum=NULL;

    pFriendList=db_search_friend_list(pIOData->ID);
    if(pFriendList==NULL)
    {
        return -1;
    }
    pFriendNum=strtok(pFriendList,";");
    while(pFriendNum!=NULL)
    {
        send_icon_to_client(pFriendNum,pIOData);
        pFriendNum=strtok(NULL,";");
    }
    free(pFriendList);

    return 0;
}

int send_group_list_to_client(IO_OPERATION_DATA *pIOData)
{
    char *pGroupList=NULL,*pGroupNum=NULL;
    GROUP_INFO_DATA groupInfoData;

    pGroupList=db_search_group_list(pIOData->ID);
    if(pGroupList==NULL)
        return -1;
    pGroupNum=strtok(pGroupList,";");
    while(pGroupNum!=NULL)
    {
        memset(&groupInfoData,NULL,sizeof(groupInfoData));
        strcat(groupInfoData.groupNum,pGroupNum);
        if(db_find_group(&groupInfoData)==0)
        {
            ex_send(pIOData,(char *)&groupInfoData,sizeof(groupInfoData),15);
        }
        pGroupNum=strtok(NULL,";");
    }
    free(pGroupList);

    return 0;
}

int send_sms_to_online_group_member(GROUP_SMS_DATA groupSMSData)
{
    char *pGroupMember=NULL,*pMemberNum=NULL;
    IO_OPERATION_DATA *pIOData=NULL;

    pGroupMember=db_search_group_member(groupSMSData.toGroupNum);
    if(pGroupMember==NULL)
        return -1;
    pMemberNum=strtok(pGroupMember,";");
    while(pMemberNum)
    {
        if(search_IO_OPERATION_DATA_by_ID(pMemberNum,&pIOData)==0)
        {
            printf("send sms to group member:%s\n",pMemberNum);
            ex_send(pIOData,(char *)&groupSMSData,sizeof(groupSMSData),16);
        }
        pMemberNum=strtok(NULL,";");
    }
    free(pGroupMember);

    return 0;
}






