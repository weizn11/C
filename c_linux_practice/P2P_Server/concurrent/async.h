#ifndef ASYNC_H_INCLUDED
#define ASYNC_H_INCLUDED

#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define LISTEN_PORT 7437
#define SOCKET_TIMEOUT 100
#define MAX_RECV_SIZE 3000
#define ASYNC_MAX_WAIT_OBJECTS 2048
#define ID_MAXIMUM_SIZE 20
#define PROTO_DATA_MAX_SIZE 2048
#define UDP_BUFFER_MAXIMUM_SIZE 65000

extern struct _IO_OPERATION_DATA_LIST_NODE_;

typedef struct
{
    int index;
    SOCKET Socket;
    SOCKET P2PTCPSocket;
    struct sockaddr_in Address;
    char *recvBuffer;
    unsigned long recvSize;
    char ID[ID_MAXIMUM_SIZE];
    struct sockaddr_in p2pAddr;
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
    //1:P2P_CONN_INFO
    unsigned int proto;    //上层协议类型
    char data[PROTO_DATA_MAX_SIZE];     //协议数据
}MAIN_PACKET;

int listen_client();

#endif // ASYNC_H_INCLUDED
