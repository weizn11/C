#ifndef P2P_H_INCLUDED
#define P2P_H_INCLUDED

#include "../concurrent/async.h"

#define P2P_UDP_LISTEN_PORT 7502
#define P2P_DATA_MAX_SIZE 2048
#define P2P_RECV_TIMEOUT 5
#define P2P_RESEND_OF_TIMES 5

typedef struct
{
    unsigned long int acknowledgementNum;    //确认号
    //unsigned int protocol;           //上层协议类型  0:校验包   1:CONN_INFO
}P2P_HEADER;

typedef struct
{
    unsigned int initFlag;
    unsigned long int preSendAcknowledgemwntNum;
    unsigned long int preRecvAcknowledgemwntNum;
}P2P_TRAN_INFO_DATA;

typedef struct
{
    int flag;
    struct sockaddr_in toAddr;
    char toID[ID_MAXIMUM_SIZE];
    char fromID[ID_MAXIMUM_SIZE];
}P2P_CONN_INFO;

typedef struct
{
    P2P_CONN_INFO connInfo;
    IO_OPERATION_DATA_NODE *pIONode;
    int index;
} P2P_CREATE_CONN_THREAD_PARA;

int p2p_create_conn_thread(void *Parameter);
int p2p_listen_server();

#endif // P2P_H_INCLUDED
