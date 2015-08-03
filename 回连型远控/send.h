#ifndef _SEND_H_
#define _SEND_H_

#include "global.h"
#include "rc4.h"

DWORD WINAPI SendOrderToClient(LPVOID Parameter);
void UploadFileToClient(gpointer Parameter);

#endif  //_SEND_H_
