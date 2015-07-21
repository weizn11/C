#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <time.h>
#include <conio.h>

#include "global.h"
#include "recv.h"

IO_OPERATION_DATA_NODE *IO_Operation_Data_Header=NULL;
unsigned long int TotalConnection=0,ListenThreads=0;
CRITICAL_SECTION CS_ListenThreads,CS_ClientList;

DWORD WINAPI listen_client(LPVOID Parameter)
{
    WSADATA wsa;
    SOCKET ServerSocket=INVALID_SOCKET,ClientSocket=INVALID_SOCKET;
    struct sockaddr_in ServerAddress,ClientAddress;
    int AddressLength,index,*ListenThreadParameter=NULL;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;

    memset(&wsa,NULL,sizeof(WSADATA));
    memset(&ServerAddress,NULL,sizeof(struct sockaddr_in));

    ServerAddress.sin_family=AF_INET;
    ServerAddress.sin_addr.s_addr=INADDR_ANY;
    ServerAddress.sin_port=htons(SERVER_LISTEN_PORT);

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("Initialization failed!\n");
        getch();
        exit(-1);
    }

    if((ServerSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
    {
        printf("Create socket failed!\n");
        getch();
        exit(-1);
    }

    if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(struct sockaddr_in))!=0)
    {
        printf("Bind local address failed\n");
        getch();
        exit(-1);
    }

    if(listen(ServerSocket,SOMAXCONN)!=0)
    {
        printf("Listen local port failed\n");
        getch();
        exit(-1);
    }
    printf("Listening...\n");
    while(1)
    {
        AddressLength=sizeof(struct sockaddr_in);
        memset(&ClientAddress,NULL,sizeof(struct sockaddr_in));
        ClientSocket=accept(ServerSocket,(struct sockaddr *)&ClientAddress,&AddressLength);
        if(ClientSocket==INVALID_SOCKET) continue;

        EnterCriticalSection(&CS_ClientList);
        if(TotalConnection==0)
        {
            IO_Operation_Data_Header=(IO_OPERATION_DATA_NODE *)malloc(sizeof(IO_OPERATION_DATA_NODE));
            if(IO_Operation_Data_Header==NULL)
            {
                printf("malloc() error!\n");
                LeaveCriticalSection(&CS_ClientList);
                return -1;
            }
            memset(IO_Operation_Data_Header,NULL,sizeof(IO_OPERATION_DATA_NODE));
        }
        else
        {
            if(TotalConnection%MAXIMUM_WAIT_OBJECTS==0)
            {
                //所有线程的任务已满
                for(IO_Operation_Node_Pointer=IO_Operation_Data_Header; IO_Operation_Node_Pointer->next!=NULL; \
                        IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next);
                IO_Operation_Node_Pointer->next=(IO_OPERATION_DATA_NODE *)malloc(sizeof(IO_OPERATION_DATA_NODE));
                if(IO_Operation_Node_Pointer->next==NULL)
                {
                    printf("malloc() error!\n");
                    LeaveCriticalSection(&CS_ClientList);
                    return -1;
                }
                memset(IO_Operation_Node_Pointer->next,NULL,sizeof(IO_OPERATION_DATA_NODE));
            }
        }
        IO_Operation_Node_Pointer=IO_Operation_Data_Header;
        while(IO_Operation_Node_Pointer->next!=NULL)
            IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next;

        index=TotalConnection%MAXIMUM_WAIT_OBJECTS;
        IO_Operation_Node_Pointer->IOArray[index]=(IO_OPERATION_DATA *)malloc(sizeof(IO_OPERATION_DATA));
        if(IO_Operation_Node_Pointer->IOArray[index]==NULL)
        {
            printf("malloc() error!\n");
            LeaveCriticalSection(&CS_ClientList);
            return -1;
        }
        memset(IO_Operation_Node_Pointer->IOArray[index],NULL,sizeof(IO_OPERATION_DATA));
        IO_Operation_Node_Pointer->IOArray[index]->Socket=ClientSocket;
        IO_Operation_Node_Pointer->IOArray[index]->overlap.hEvent=IO_Operation_Node_Pointer->EventArray[index]=WSACreateEvent();
        IO_Operation_Node_Pointer->IOArray[index]->WSABuffer.buf=IO_Operation_Node_Pointer->IOArray[index]->RecvBuffer;
        IO_Operation_Node_Pointer->IOArray[index]->WSABuffer.len=MAX_SIZE;
        IO_Operation_Node_Pointer->IOArray[index]->Address=ClientAddress;
        WSARecv(ClientSocket,&IO_Operation_Node_Pointer->IOArray[index]->WSABuffer,1,&IO_Operation_Node_Pointer->IOArray[index]->RecvSize,\
                &IO_Operation_Node_Pointer->IOArray[index]->flag,&IO_Operation_Node_Pointer->IOArray[index]->overlap,NULL);
        TotalConnection++;
        if((TotalConnection-1)%MAXIMUM_WAIT_OBJECTS==0)
        {
            //创建新的监听线程
            CloseHandle(CreateThread(NULL,0,recv_message_from_client,NULL,0,NULL));
        }
        printf("Client \"%s\" online.TotalConnect:%d\n",inet_ntoa(ClientAddress.sin_addr),TotalConnection);
        LeaveCriticalSection(&CS_ClientList);
    }

    return 0;
}

int main(int argc,char *argv[])
{
    InitializeCriticalSection(&CS_ListenThreads);
    InitializeCriticalSection(&CS_ClientList);

    CloseHandle(CreateThread(NULL,0,listen_client,NULL,0,NULL));

    while(1) Sleep(1000);

    return 0;
}









