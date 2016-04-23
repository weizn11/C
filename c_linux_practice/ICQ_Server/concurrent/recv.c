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
    char *recvPointer=NULL,*pFriendList=NULL,*pFriendNum=NULL;
    char str[MAX_RECV_SIZE];
    MAIN_PACKET *pPacket=NULL;
    P2P_CONN_INFO *pConnInfo=NULL;
    P2P_CREATE_CONN_THREAD_PARA *pP2PCreateConnThreadPara=NULL;
    pthread_t threadID;
    ACCOUNT_INFO_DATA accountInfo;
    FRIENDSHIP_INFO_DATA friendshipInfo;
    IO_OPERATION_DATA *pTmpIOData=NULL;
    FRIEND_INFO_DATA friendInfoData;
    ICON_TRAN_DATA *pIconData=NULL;
    SMS_DATA smsData;
    FILE_TRAN_DATA *pFileTranData=NULL;

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
        printf("Recv proto:%d\n",pPacket->proto);
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
        case 2:
            //auth_account
            memset(&accountInfo,NULL,sizeof(ACCOUNT_INFO_DATA));
            accountInfo=*(ACCOUNT_INFO_DATA *)pPacket->data;
            if(search_IO_OPERATION_DATA_by_ID(accountInfo.num,NULL)==0)
            {
                printf("User '%s' is already online.\n",accountInfo.num);
                accountInfo.success=-1;      //already online
            }
            else
            {
                if(db_auth_account(&accountInfo)==0)
                {
                    printf("User '%s' auth successful.\n",accountInfo.num);
                    memset(pIOData->ID,NULL,sizeof(pIOData->ID));
                    strcat(pIOData->ID,accountInfo.num);
                    accountInfo.success=1;
                    pIOData->onlineStatus=1;
                }
                else
                    printf("User '%s' auth failed.\n",accountInfo.num);
            }
            ex_send(pIOData,(char *)&accountInfo,sizeof(accountInfo),2);

            if(accountInfo.success!=1) break;
            send_icon_to_client(pIOData->ID,pIOData);
            send_friend_icon_to_me(pIOData);
            pFriendList=db_search_friend_list(accountInfo.num);
            if(pFriendList==NULL) break;
            pFriendNum=strtok(pFriendList,";");
            while(pFriendNum!=NULL)
            {
                printf("find friend '%s'\n",pFriendNum);
                if(search_IO_OPERATION_DATA_by_ID(pFriendNum,&pTmpIOData)==0)
                {
                    memset(&friendInfoData,NULL,sizeof(friendInfoData));
                    strcat(friendInfoData.friendInfo.num,accountInfo.num);
                    db_find_friend(&friendInfoData.friendInfo);
                    friendInfoData.onlineStatus=1;
                    ex_send(pTmpIOData,(char *)&friendInfoData,sizeof(friendInfoData),6);
                    memset(&friendInfoData,NULL,sizeof(friendInfoData));
                    friendInfoData.onlineStatus=1;
                }
                else
                {
                    memset(&friendInfoData,NULL,sizeof(friendInfoData));
                    friendInfoData.onlineStatus=0;
                }
                strcat(friendInfoData.friendInfo.num,pFriendNum);
                db_find_friend(&friendInfoData.friendInfo);
                ex_send(pIOData,(char *)&friendInfoData,sizeof(friendInfoData),6);
                pFriendNum=strtok(NULL,";");
            }
            free(pFriendList);
            send_friend_icon_to_me(pIOData);
            memset(&smsData,NULL,sizeof(smsData));
            strcat(smsData.toNum,pIOData->ID);
            while(db_query_leave_message(&smsData)==0)
            {
                ex_send(pIOData,(char *)&smsData,sizeof(smsData),9);
            }
            send_group_list_to_client(pIOData);
            break;
        case 3:
            //register account
            if(db_register_account(((REGISTER_ACCOUNT_DATA *)(pPacket->data))->num,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->nickname,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->password,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->truename,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->school,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->sex,\
                                   ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->age)==0)
            {
                //new account register successful
                ((REGISTER_ACCOUNT_DATA *)(pPacket->data))->success=1;
            }
            ex_send(pIOData,(char *)pPacket->data,sizeof(REGISTER_ACCOUNT_DATA),3);
            break;
        case 4:
            //find friend
            db_find_friend((ACCOUNT_INFO_DATA *)pPacket->data);
            ex_send(pIOData,(char *)pPacket->data,sizeof(ACCOUNT_INFO_DATA),4);
            break;
        case 5:
            //add friendship
            if(!strcmp(((FRIENDSHIP_INFO_DATA *)pPacket->data)->num2,((FRIENDSHIP_INFO_DATA *)pPacket->data)->num1))
            {
                ((FRIENDSHIP_INFO_DATA *)pPacket->data)->success=-3;
                goto friendship_skip;
            }
            if((((FRIENDSHIP_INFO_DATA *)pPacket->data)->success=\
                    db_add_friendship((FRIENDSHIP_INFO_DATA *)pPacket->data))==1)
            {
                //add friendship success
                ex_send(pIOData,pPacket->data,sizeof(FRIENDSHIP_INFO_DATA),5);
                memset(&friendInfoData,NULL,sizeof(friendInfoData));
                strcat(friendInfoData.group,((FRIENDSHIP_INFO_DATA *)pPacket->data)->group1);
                strcat(friendInfoData.friendInfo.num,((FRIENDSHIP_INFO_DATA *)pPacket->data)->num2);
                db_find_friend(&friendInfoData.friendInfo);
                if(search_IO_OPERATION_DATA_by_ID(friendInfoData.friendInfo.num,&pTmpIOData)==0)
                        friendInfoData.onlineStatus=pTmpIOData->onlineStatus;
                else
                    friendInfoData.onlineStatus=0;
                ex_send(pIOData,(char *)&friendInfoData,sizeof(FRIEND_INFO_DATA),6);
                send_icon_to_client(friendInfoData.friendInfo.num,pIOData);
                if(friendInfoData.onlineStatus)
                {
                    printf("'%s' online\n",pTmpIOData->ID);
                    memset(&friendInfoData,NULL,sizeof(friendInfoData));
                    friendInfoData.onlineStatus=pTmpIOData->onlineStatus;
                    strcat(friendInfoData.friendInfo.num,pIOData->ID);
                    strcat(friendInfoData.group,((FRIENDSHIP_INFO_DATA *)pPacket->data)->group2);
                    db_find_friend(&friendInfoData.friendInfo);
                    ex_send(pTmpIOData,(char *)&friendInfoData,sizeof(FRIEND_INFO_DATA),6);
                    send_icon_to_client(pIOData->ID,pTmpIOData);
                }
            }
            else
            {
friendship_skip:
                ex_send(pIOData,pPacket->data,sizeof(FRIENDSHIP_INFO_DATA),5);
            }
            break;
        case 6:
            pIOData->onlineStatus=((FRIEND_INFO_DATA *)pPacket->data)->onlineStatus;
            broadcast_data_to_friend(pIOData,pPacket->data,sizeof(FRIEND_INFO_DATA),6);
            break;
        case 7:
            //update account data
            memset(((ACCOUNT_INFO_DATA *)pPacket->data)->num,NULL,ID_MAXIMUM_SIZE);
            strcat(((ACCOUNT_INFO_DATA *)pPacket->data)->num,pIOData->ID);
            if(db_update_account_data(pPacket->data)==0)
                ((ACCOUNT_INFO_DATA *)pPacket->data)->success=1;
            else
                ((ACCOUNT_INFO_DATA *)pPacket->data)->success=-1;
            ex_send(pIOData,pPacket->data,sizeof(ACCOUNT_INFO_DATA),7);
            memset(&friendInfoData,NULL,sizeof(friendInfoData));
            friendInfoData.friendInfo=*((ACCOUNT_INFO_DATA *)pPacket->data);
            friendInfoData.onlineStatus=pIOData->onlineStatus;
            broadcast_data_to_friend(pIOData,(char *)&friendInfoData,sizeof(friendInfoData),6);
            break;
        case 8:
            //ICON_TRAN_DATA
            pIconData=(ICON_TRAN_DATA *)pPacket->data;
            if(pIOData->pFileIcon==NULL)
            {
                sprintf(str,"images/icon/%s.jpg",pIOData->ID);
                pIOData->pFileIcon=fopen(str,"wb");
                if(pIOData->pFileIcon==NULL)
                {
                    printf("create file %s failed.\n",str);
                    break;
                }
                fwrite(pIconData->data,pIconData->dataSize,1,pIOData->pFileIcon);
            }
            else
            {
                fwrite(pIconData->data,pIconData->dataSize,1,pIOData->pFileIcon);
            }
            if(pIconData->flag==0)
            {
                if(pIOData->pFileIcon!=NULL)
                    fclose(pIOData->pFileIcon);
                pIOData->pFileIcon=NULL;
                printf("'%s''s icon upload finish.\n",pIOData->ID);
                broadcast_icon_to_friend(pIOData);
                printf("broadcast icon to friends successful.\n");
            }
            break;
        case 9:
            //SMS_DATA
            ((SMS_DATA *)pPacket->data)->sms[1999]=NULL;
            ((SMS_DATA *)pPacket->data)->fromNum[ID_MAXIMUM_SIZE-1]=NULL;
            ((SMS_DATA *)pPacket->data)->toNum[ID_MAXIMUM_SIZE-1]=NULL;
            if(search_IO_OPERATION_DATA_by_ID(((SMS_DATA *)pPacket->data)->toNum,&pTmpIOData)==0)
            {
                //the friend online
                printf("mapping sms.\n");
                ex_send(pTmpIOData,pPacket->data,sizeof(SMS_DATA),9);
            }
            else
            {
                printf("add leave message.\n");
                db_add_leave_message(*((SMS_DATA *)pPacket->data));
            }
            break;
        case 10:
            //FILE_TRAN_DATA
            pFileTranData=(FILE_TRAN_DATA *)pPacket->data;
            if(search_IO_OPERATION_DATA_by_ID(pFileTranData->toNum,&pTmpIOData)==0 && pTmpIOData->onlineStatus==1)
            {
                //printf("friend online.\n");
                ex_send(pTmpIOData,pFileTranData,sizeof(FILE_TRAN_DATA),10);
            }
            else
            {
                //printf("user '%s' is offline.\n",pFileTranData->toNum);
                pFileTranData->auth=-1;     //toNum if offline
                ex_send(pIOData,(char *)pFileTranData,sizeof(FILE_TRAN_DATA),10);
            }
            break;
        case 11:
            //delete friend
            db_delete_friend(((FRIENDSHIP_INFO_DATA *)pPacket->data)->num1,((FRIENDSHIP_INFO_DATA *)pPacket->data)->num2);
            if(search_IO_OPERATION_DATA_by_ID(((FRIENDSHIP_INFO_DATA *)pPacket->data)->num2,&pTmpIOData)==0)
            {
                ex_send(pTmpIOData,pPacket->data,sizeof(FRIENDSHIP_INFO_DATA),11);
            }
            break;
        case 12:
            //create group
            if(db_create_group(pPacket->data)==0)
            {
                printf("Create group '%s' successful.",((CREATE_GROUP_DATA *)pPacket->data)->groupNum);
                ((CREATE_GROUP_DATA *)pPacket->data)->flag=1;
                ex_send(pIOData,pPacket->data,sizeof(CREATE_GROUP_DATA),12);
                db_join_group((GROUP_INFO_DATA *)pPacket->data,pIOData->ID);
                send_group_list_to_client(pIOData);
            }
            else
            {
                printf("Create group '%s' failed.",((CREATE_GROUP_DATA *)pPacket->data)->groupNum);
                ex_send(pIOData,pPacket->data,sizeof(CREATE_GROUP_DATA),12);
            }
            break;
        case 13:
            //find_group
            db_find_group((GROUP_INFO_DATA *)pPacket->data);
            ex_send(pIOData,pPacket->data,sizeof(GROUP_INFO_DATA),13);
            break;
        case 14:
            //join_group
            db_join_group((GROUP_INFO_DATA *)pPacket->data,pIOData->ID);
            ex_send(pIOData,(char *)pPacket->data,sizeof(GROUP_INFO_DATA),14);
            send_group_list_to_client(pIOData);
            break;
        case 16:
            //GROUP_SMS_DATA
            send_sms_to_online_group_member(*(GROUP_SMS_DATA *)pPacket->data);
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










