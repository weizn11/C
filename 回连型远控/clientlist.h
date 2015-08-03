#ifndef _CLIENTLIST_H_
#define _CLIENTLIST_H_

#include "global.h"

int GetIndexBySocket(SOCKET ClientSocket);
SOCKET GetClientSocket(int n);
int AppendCmdRecord(int index,char *cmd);
int AddClientList(ClientListNode *CLN,SOCKET ClientSocket);
int CleanupClientConnection(SOCKET ClientSocket);



#endif   //_CLIENTLIST_H_