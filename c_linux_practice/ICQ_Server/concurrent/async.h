#ifndef ASYNC_H_INCLUDED
#define ASYNC_H_INCLUDED

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define LISTEN_PORT 7437
#define SOCKET_TIMEOUT 100
#define MAX_RECV_SIZE 3000
#define ASYNC_MAX_WAIT_OBJECTS 2048
#define ID_MAXIMUM_SIZE 20
#define PASSWORD_MAXIMUM_SIZE 30
#define PROTO_DATA_MAX_SIZE 2048
#define UDP_BUFFER_MAXIMUM_SIZE 65000

extern struct _IO_OPERATION_DATA_LIST_NODE_;

typedef struct
{
    int index;
    int onlineStatus;    //0:hide   1:online
    SOCKET Socket;
    SOCKET P2PTCPSocket;
    struct sockaddr_in Address;
    char *recvBuffer;
    unsigned long recvSize;
    char ID[ID_MAXIMUM_SIZE];
    struct sockaddr_in p2pAddr;
    FILE *pFileIcon;
    pthread_mutex_t sendMutex;
    struct _IO_OPERATION_DATA_LIST_NODE_ *pIONode;
} IO_OPERATION_DATA;

typedef struct _IO_OPERATION_DATA_LIST_NODE_
{
    int epollfd;
    IO_OPERATION_DATA *IOArray[ASYNC_MAX_WAIT_OBJECTS];
    struct _IO_OPERATION_DATA_LIST_NODE_ *next;
} IO_OPERATION_DATA_NODE;

typedef struct
{
    int success;
    char num[ID_MAXIMUM_SIZE];
    char nickname[100];
    char truename[100];
    char school[100];
    char password[30];
    char signature[140];
    char groupList[500];
    int sex;
    int age;
}REGISTER_ACCOUNT_DATA,ACCOUNT_INFO_DATA;

typedef struct
{
    int onlineStatus;    //1:online     0:offline
    ACCOUNT_INFO_DATA friendInfo;
    char group[50];
}FRIEND_INFO_DATA;

typedef struct
{
    int success;
    char num1[ID_MAXIMUM_SIZE];
    char num2[ID_MAXIMUM_SIZE];
    char group1[50];
    char group2[50];
}FRIENDSHIP_INFO_DATA;

typedef struct
{
    int flushFlag;
    int flag;   //1:continue    0:finish
    int dataSize;
    char num[ID_MAXIMUM_SIZE];
    char data[1024];
}ICON_TRAN_DATA;

typedef struct
{
    char fromNum[ID_MAXIMUM_SIZE];
    char toNum[ID_MAXIMUM_SIZE];
    char sms[2000];
}SMS_DATA;

typedef struct
{
    unsigned int sequenceNum;   //序列号
    int auth;           //1:通过许可    0:传输拒绝
    int continueFlag;      //1:continue    0:finish
    unsigned long fileSize;
    int dataSize;
    char fromNum[ID_MAXIMUM_SIZE];
    char toNum[ID_MAXIMUM_SIZE];
    char fileName[255];
    char fileData[1500];
}FILE_TRAN_DATA;

typedef struct
{
    int flag;
    char groupNum[ID_MAXIMUM_SIZE];
    char groupName[100];
    char groupIntro[200];
}CREATE_GROUP_DATA,GROUP_INFO_DATA;

typedef struct
{
    char toGroupNum[ID_MAXIMUM_SIZE];
    ACCOUNT_INFO_DATA fromFriendInfo;
    char sms[1000];
}GROUP_SMS_DATA;

typedef struct
{
    /*
     1:P2P_CONN_INFO  2:auth_account() ACCOUNT_INFO_DATA  3:REGISTER_ACCOUNT_DATA  4:db_find_find(),ACCOUNT_INFO_DATA
     5:add_friend() FRIENDSHIP_INFO_DATA    6:update_friend_item FRIEND_INFO_DATA
     7:update_account_data() ACCOUNT_INFO_DATA  8:ICON_TRAN_DATA
     9:SMS_DATA   10:FILE_TRAN_DATA 11:db_delete_friend() FRIENDSHIP_INFO_DATA
     12:CREATE_GROUP_DATA   13:find_group() GROUP_INFO_DATA  14:join_group() GROUP_INFO_DATA
     15:update client groups item GROUP_INFO_DATA  16:GROUP_SMS_DATA
    */
    unsigned int proto;    //上层协议类型
    char data[PROTO_DATA_MAX_SIZE];     //协议数据
}MAIN_PACKET;

int listen_client();
int ex_send(IO_OPERATION_DATA *pIOData,char *sendBuffer,unsigned int bufferSize,int proto);
int search_IO_OPERATION_DATA_by_ID(char *ID,IO_OPERATION_DATA **ppIOData);

#endif // ASYNC_H_INCLUDED
