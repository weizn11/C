#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <winsock2.h>

#define SERVER_LISTEN_PORT 9999       //服务器监听端口
#define MAX_SIZE 65535

typedef struct _IO_OPERATION_DATA_
{
    //重叠结构
    SOCKET Socket;
    struct sockaddr_in Address;
    WSAOVERLAPPED overlap;
    WSABUF WSABuffer;
    char RecvBuffer[MAX_SIZE+1];
    DWORD RecvSize;
    DWORD flag;
}IO_OPERATION_DATA;

typedef struct _IO_OPERATION_DATA_LIST_NODE_
{
    IO_OPERATION_DATA *IOArray[MAXIMUM_WAIT_OBJECTS];
    WSAEVENT EventArray[MAXIMUM_WAIT_OBJECTS];
    struct _IO_OPERATION_DATA_LIST_NODE_ *next;
}IO_OPERATION_DATA_NODE;

#endif // GLOBAL_H_INCLUDED







