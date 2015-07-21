#include "global.h"
#include "recv.h"

extern IO_OPERATION_DATA_NODE *IO_Operation_Data_Header;
extern unsigned long int TotalConnection,ListenThreads;
extern CRITICAL_SECTION CS_ListenThreads,CS_ClientList;

int cleanup_client_connection(int ThreadIndex,int EventIndex)
{
    int i=ThreadIndex;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL,*IO_Operation_Last_Pointer=NULL;

    EnterCriticalSection(&CS_ClientList);
    IO_Operation_Node_Pointer=IO_Operation_Data_Header;
    while(--i)
        IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next;

    printf("Client \"%s\" logged off.TotalConnect:%d\n",inet_ntoa(IO_Operation_Node_Pointer->IOArray[EventIndex]->Address.sin_addr),\
           TotalConnection-1);

    closesocket(IO_Operation_Node_Pointer->IOArray[EventIndex]->Socket);
    WSACloseEvent(IO_Operation_Node_Pointer->EventArray[EventIndex]);
    free(IO_Operation_Node_Pointer->IOArray[EventIndex]);
    IO_Operation_Node_Pointer->IOArray[EventIndex]=NULL;

    IO_Operation_Last_Pointer=IO_Operation_Node_Pointer;
    while(IO_Operation_Last_Pointer->next!=NULL)
        IO_Operation_Last_Pointer=IO_Operation_Last_Pointer->next;

    i=(TotalConnection-1)%MAXIMUM_WAIT_OBJECTS;
    IO_Operation_Node_Pointer->EventArray[EventIndex]=IO_Operation_Last_Pointer->EventArray[i];
    IO_Operation_Node_Pointer->IOArray[EventIndex]=IO_Operation_Last_Pointer->IOArray[i];
    IO_Operation_Last_Pointer->IOArray[i]=NULL;
    if(i==0)
    {
        IO_Operation_Node_Pointer=IO_Operation_Data_Header;
        while(IO_Operation_Node_Pointer->next!=NULL && IO_Operation_Node_Pointer->next!=IO_Operation_Last_Pointer)
            IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next;

        if(IO_Operation_Node_Pointer->next==NULL)
        {
            free(IO_Operation_Data_Header);
            IO_Operation_Data_Header=NULL;
        }
        else
        {
            free(IO_Operation_Node_Pointer->next);
            IO_Operation_Node_Pointer->next=NULL;
        }
    }
    TotalConnection--;
    LeaveCriticalSection(&CS_ClientList);

    return 0;
}

DWORD WINAPI recv_message_from_client(LPVOID Parameter)
{
    int i,ThreadIndex,ClientCount,index;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;

    EnterCriticalSection(&CS_ListenThreads);
    ListenThreads++;
    i=ThreadIndex=ListenThreads;
    LeaveCriticalSection(&CS_ListenThreads);

    IO_Operation_Node_Pointer=IO_Operation_Data_Header;
    while(--i)
        IO_Operation_Node_Pointer=IO_Operation_Node_Pointer->next;

    while(1)
    {
        if(ListenThreads>ThreadIndex)
            ClientCount=MAXIMUM_WAIT_OBJECTS;
        else
            ClientCount=TotalConnection-(ThreadIndex-1)*MAXIMUM_WAIT_OBJECTS;
        if(ClientCount<=0)
        {
            //线程退出
            EnterCriticalSection(&CS_ListenThreads);
            ListenThreads--;
            LeaveCriticalSection(&CS_ListenThreads);
            return 0;
        }

        index=WSAWaitForMultipleEvents(ClientCount,IO_Operation_Node_Pointer->EventArray,FALSE,1,FALSE);
        if(index==WSA_WAIT_FAILED || index==WSA_WAIT_TIMEOUT)
        {
            if(ListenThreads>ThreadIndex)
                ClientCount=MAXIMUM_WAIT_OBJECTS;
            else
                ClientCount=TotalConnection-(ThreadIndex-1)*MAXIMUM_WAIT_OBJECTS;

            if(ClientCount<=0)
            {
                //线程退出
                EnterCriticalSection(&CS_ListenThreads);
                ListenThreads--;
                LeaveCriticalSection(&CS_ListenThreads);
                return 0;
            }
            continue;
        }

        index-=WSA_WAIT_EVENT_0;    //获取事件发生的对应索引
        WSAResetEvent(IO_Operation_Node_Pointer->EventArray[index]);    //重置事件
        WSAGetOverlappedResult(IO_Operation_Node_Pointer->IOArray[index]->Socket,&IO_Operation_Node_Pointer->IOArray[index]->overlap,\
                               &IO_Operation_Node_Pointer->IOArray[index]->RecvSize,TRUE,&IO_Operation_Node_Pointer->IOArray[index]->flag);
        if(IO_Operation_Node_Pointer->IOArray[index]->RecvSize<=0)
        {
            //客户端退出
            cleanup_client_connection(ThreadIndex,index);
            continue;
        }

        //处理RecvBuffer



        memset(&IO_Operation_Node_Pointer->IOArray[index]->RecvBuffer,NULL,MAX_SIZE+1);
        WSARecv(IO_Operation_Node_Pointer->IOArray[index]->Socket,&IO_Operation_Node_Pointer->IOArray[index]->WSABuffer,1,\
                &IO_Operation_Node_Pointer->IOArray[index]->RecvSize,&IO_Operation_Node_Pointer->IOArray[index]->flag,\
                &IO_Operation_Node_Pointer->IOArray[index]->overlap,NULL);
    }
    return 0;
}








