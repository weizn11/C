#ifndef _RECV_H_
#define _RECV_H_

#include "global.h"
#include "rc4.h"

void RecvMessageFromClient(gpointer Parameter);
void CheckHTTPOnlineClient();
void RecvFile(gpointer Parameter);
int RecvClientInfo(SOCKET ClientSocket,ClientInfo *CI);
int dispose_http_recv(char *RecvBuffer,char **buffer,int *write_len,int *sur_len,int recv_size);

#endif    //_RECV_H_

