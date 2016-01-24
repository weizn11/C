#ifndef ASYNC_H_INCLUDED
#define ASYNC_H_INCLUDED

#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define LISTEN_PORT 7437
#define SOCKET_TIMEOUT 100      //epoll_wait超时时间
#define MAX_RECV_SIZE 2048
#define ASYNC_MAX_WAIT_OBJECTS 2048     //一个监听线程最大服务的对象数量

/*
IO_OPERATION_DATA结构体用来存放每个客户端连接的相关信息及数据
Socket:与客户端连接的套接字
Address:客户端地址
recvBuffer:指向存放接收到客户端传来数据的内存
recvSize:当前recvBuffer已接收到的数据长度
*/
typedef struct _IO_OPERATION_DATA_LIST_NODE_ IO_OPERATION_DATA_NODE;
typedef struct
{
    SOCKET Socket;
    struct sockaddr_in Address;
    int posIndex;
    IO_OPERATION_DATA_NODE *pIONode;
    char *recvBuffer;
    unsigned long recvSize;
} IO_OPERATION_DATA;

/*
IO_OPERATION_DATA_NODE结构体用来保存一组客户端信息结构体和epoll描述字，也是一个线程基本的操作单位
epollfd:epoll描述字
IOArray:存放指向IO_OPERATION_DATA的指针数组
next:指向下一个链表节点
*/
typedef struct _IO_OPERATION_DATA_LIST_NODE_
{
    int epollfd;
    IO_OPERATION_DATA *IOArray[ASYNC_MAX_WAIT_OBJECTS];
    struct _IO_OPERATION_DATA_LIST_NODE_ *next;
} IO_OPERATION_DATA_NODE;

#endif // ASYNC_H_INCLUDED
