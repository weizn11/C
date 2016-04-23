#ifndef ASYNC_H_INCLUDED
#define ASYNC_H_INCLUDED

#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define LISTEN_PORT 7437
#define SOCKET_TIMEOUT 100
#define MAX_RECV_SIZE 2048
#define ASYNC_MAX_WAIT_OBJECTS 2048

typedef struct
{
    SOCKET Socket;
    struct sockaddr_in Address;
    char *recvBuffer;
    unsigned long recvSize;
} IO_OPERATION_DATA;

typedef struct _IO_OPERATION_DATA_LIST_NODE_
{
    int epollfd;
    IO_OPERATION_DATA *IOArray[ASYNC_MAX_WAIT_OBJECTS];
    struct _IO_OPERATION_DATA_LIST_NODE_ *next;
} IO_OPERATION_DATA_NODE;

#endif // ASYNC_H_INCLUDED
