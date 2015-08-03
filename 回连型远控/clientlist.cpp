#include "clientlist.h"

int GetIndexBySocket(SOCKET ClientSocket)
{
    int Index=0;

    for (Index=0; Index<100*MAXIMUM_WAIT_OBJECTS; Index++)
    {
        if(ClientSocketArray[Index]==ClientSocket)
            return Index;
    }

    return -1;
}


SOCKET GetClientSocket(int n)
{
    SOCKET TargetClient=INVALID_SOCKET;
    ClientListNode *List_Node=NULL;
	
    for(List_Node=List_Header; List_Node; n--,List_Node=List_Node->next)
        if(n==1)
        {
            TargetClient=List_Node->soc;
            break;
        }
		
		return TargetClient;
}

int AppendCmdRecord(int index,char *cmd)
{
	CommandRecordNode *temp=NULL,*body=NULL,*pre=NULL;

	if((temp=(CommandRecordNode *)malloc(sizeof(CommandRecordNode)))==NULL)
	{
		MessageBox(NULL,"malloc error","ERROR",MB_OK);
		exit(-1);
	}
	memset(temp,NULL,sizeof(CommandRecordNode));

	if((temp->cmd=(char *)malloc((strlen(cmd)+1)*sizeof(char)))==NULL)
	{
		MessageBox(NULL,"malloc error","ERROR",MB_OK);
		exit(-1);
	}
	memset(temp->cmd,NULL,(strlen(cmd)+1)*sizeof(char));
	strcat(temp->cmd,cmd);
    if (IODataArray[index]->cmdrecord==NULL)
    {
		IODataArray[index]->cmdrecord=temp;
    }
    else
    {
		for(pre=body=IODataArray[index]->cmdrecord;body->next!=NULL;body=body->next) pre=body;
		body->next=temp;
		temp->front=pre;
    }

	for(body=IODataArray[index]->cmdrecord;body->next!=NULL;body=body->next);
	IODataArray[index]->cmdrecord->front=body;

    return 0;
}

int AddClientList(ClientListNode *CLN,SOCKET ClientSocket)
{
    ClientListNode *List_Node=NULL,*New_Node=NULL;

    if((New_Node=(ClientListNode *)malloc(sizeof(ClientListNode)))==NULL)
    {
        //申请节点空间失败
        return -1;
    }
    memset(New_Node,NULL,sizeof(ClientListNode));
    if(List_Header==NULL)
    {
        List_Header=New_Node;      //若客户端链表为空则重建链表
    }
    else
    {
        List_Node=List_Header;
        while(List_Node->next)
            List_Node=List_Node->next;
        List_Node->next=New_Node;
    }

    CLN->soc=ClientSocket;
    *New_Node=*CLN;
    New_Node->next=NULL;

	EnterCriticalSection(&CRI_update_list);
	gdk_threads_enter();
	add_to_list(*CLN);
	gdk_threads_leave();
	LeaveCriticalSection(&CRI_update_list);

    return 0;
}

int CleanupClientConnection(SOCKET ClientSocket)
{
    int index=0,list_index=0;
    ClientListNode *List_Node=NULL,*tmp=NULL;
	CommandRecordNode *CRN_body=NULL,*CRN_temp=NULL;

	if(ClientSocket==INVALID_SOCKET)
		return 0;
    index=GetIndexBySocket(ClientSocket);
	//printf("Index:%d\n",index);
	closesocket(ClientSocket);
    ClientSocketArray[index]=INVALID_SOCKET;
    EnterCriticalSection(&CRI_List);
    if(List_Header==NULL) return -1;
    List_Node=List_Header;
    if(List_Header->soc==ClientSocket)
    {
        //链表头节点将被删除
        List_Header=List_Header->next;
        free(List_Node);                   //释放空间
		list_index=0;
    }
    else
    {
        while(List_Node)
        {
			list_index++;
            if(List_Node->next->soc==ClientSocket)
            {
                tmp=List_Node->next;
                List_Node->next=List_Node->next->next;
                free(tmp);
                break;
            }
            List_Node=List_Node->next;
        }
    }
    LeaveCriticalSection(&CRI_List);

	for (CRN_body=IODataArray[index]->cmdrecord;CRN_body!=NULL;)
	{
		free(CRN_body->cmd);
		CRN_temp=CRN_body;
		CRN_body=CRN_body->next;
		free(CRN_temp);
	}

    memset(IODataArray[index],NULL,sizeof(IO_OPERATION_DATA));
    free(IODataArray[index]);
    WSACloseEvent(WSAEventArray[index]);
    if (index<TotalConnection-1)
    {
        ClientSocketArray[index]=ClientSocketArray[TotalConnection-1];
        IODataArray[index]=IODataArray[TotalConnection-1];
        WSAEventArray[index]=WSAEventArray[TotalConnection-1];
    }
    EnterCriticalSection(&CRI_Total);
    TotalConnection--;
    LeaveCriticalSection(&CRI_Total);

	EnterCriticalSection(&CRI_update_list);
	gdk_threads_enter();
	remove_item(list_index);
	gdk_threads_leave();
	LeaveCriticalSection(&CRI_update_list);

    return 0;
}