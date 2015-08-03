#include "send.h"

void UploadFileToClient(gpointer Parameter)
{
    FILE *file=NULL;
    char SendBuffer[MAX_SIZE],ReadBuffer[MAX_SIZE],*pStart=NULL;
    ControlClient _CC=UpCon;
    int PacketSize,n;
    unsigned long TotalSize,acc;
	SOCKET soc;
	MessageStruct MS;

    memset(SendBuffer,NULL,sizeof(SendBuffer));
    sprintf(SendBuffer,"Upload_File:%s",_CC.remotepath);
    if (_CC.HTTP_Proxy)
    {
        //通过HTTP代理发送文件
        if(Send_HTTP_Header(_CC.soc,sizeof(SendBuffer))!=0)
        {
            closesocket(_CC.soc);
            Upload_Socket=INVALID_SOCKET;
            g_thread_exit(NULL);
        }
		Sleep(1000);
    }
	rc4_crypt(S_box,(unsigned char *)SendBuffer,sizeof(SendBuffer));
    if(send(_CC.soc,SendBuffer,sizeof(SendBuffer),0)<=0)
    {
		//printf("发送上传信息失败\n");
        CleanupClientConnection(_CC.soc);
        g_thread_exit(NULL);
    }
    for (n=0; UploadFileFlag==0 && n<=10000; n++)
    {
        //等待客户端允许上传消息，超时时间10秒
        Sleep(1);
    }
    if (UploadFileFlag!=1)
    {
        //上传失败
        g_thread_exit(NULL);
    }
    UploadFileFlag=0;            //重置文件上传标识

	for (n=0; Upload_Socket==INVALID_SOCKET && n<=10000; n++)
    {
        //等待客户端建立文件上传套接字，超时时间10秒
        Sleep(1);
    }
    if(Upload_Socket==INVALID_SOCKET)
    {
		memset(&MS,NULL,sizeof(MessageStruct));
		strcat(MS.title,"Error");
		strcat(MS.content,"Socket time out!");
		CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        g_thread_exit(NULL);
    }
	soc=Upload_Socket;
	Upload_Socket=INVALID_SOCKET;

    if ((file=fopen(_CC.localpath,"rb"))==NULL)
    {
		memset(&MS,NULL,sizeof(MessageStruct));
		strcat(MS.title,"Error");
		strcat(MS.content,"Open local file failed!");
		CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
		closesocket(soc);
        g_thread_exit(NULL);
    }
    fseek(file,0,SEEK_END);
    TotalSize=ftell(file);        //获取文件总大小
    fseek(file,0,SEEK_SET);

	//printf("开始上传文件。\n");
    acc=0;
    while(!feof(file))
    {
        memset(SendBuffer,NULL,sizeof(SendBuffer));
        memset(ReadBuffer,NULL,sizeof(ReadBuffer));

        PacketSize=fread(SendBuffer,sizeof(char),sizeof(SendBuffer),file);
        if(PacketSize<1)
			continue;

        if(send(soc,SendBuffer,PacketSize,0)<=0)
        {
            closesocket(soc);
            soc=INVALID_SOCKET;
            break;
        }
        acc+=PacketSize;
        gdk_threads_enter();
        update_progressbar(_CC.pbar,TotalSize,acc,1);
        gdk_threads_leave();
        //	printf("PacketSize:%d\n",PacketSize);
    }
    //printf("文件:%s上传成功.\n",_CC.localpath);
// 	memset(&MS,NULL,sizeof(MessageStruct));
// 	strcat(MS.title,"Success");
// 	strcat(MS.content,"Upload file successfully!");
//  CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
    closesocket(soc);
    soc=INVALID_SOCKET;
    fclose(file);
    g_thread_exit(NULL);
}

int Send_HTTP_Header(SOCKET ProxySocket,int Content_Length)
{
    char SendBuffer[MAX_SIZE];

    memset(SendBuffer,NULL,sizeof(SendBuffer));
    sprintf(SendBuffer,HTTP_Header,Content_Length);
    if(send(ProxySocket,SendBuffer,strlen(SendBuffer),0)<=0)
    {
        //printf("客户端断开.");
        CleanupClientConnection(ProxySocket);
        return -1;
    }
	Sleep(100);    //等待HTTP_Header发送完成
    return 0;
}

DWORD WINAPI SendOrderToClient(LPVOID Parameter)
{
    int Index=0,SendSize=0;
    char SendBuffer[MAX_SIZE],sendflag=0;

    while(1)
    {
        if(CC.soc!=INVALID_SOCKET)
        {
            //服务端将发送控制命令
            memset(SendBuffer,NULL,sizeof(SendBuffer));
            switch(CC.order)
            {
            case 1:       //远程执行命令
                strcat(SendBuffer,"Execute_Command:");
                sendflag=1;
                break;
            case 2:      //上传文件
				memset(&UpCon,NULL,sizeof(ControlClient));
                UpCon=CC;
                g_thread_create((GThreadFunc)UploadFileToClient,NULL,TRUE,NULL);         //创建文件上传线程
                break;
            case 3:      //下载文件
                memset(&DownCon,NULL,sizeof(ControlClient));
                DownCon=CC;
                strcat(SendBuffer,"Download_File:");
                strcat(SendBuffer,CC.remotepath);
                Index=GetIndexBySocket(CC.soc);
                memset(IODataArray[Index]->download_path,NULL,255);
                strcat(IODataArray[Index]->download_path,CC.localpath);
                sendflag=1;
                break;
            case 4:     //断开连接
                strcat(SendBuffer,"Disconnect:");
                sendflag=1;
                break;
			case 5:     //文件浏览
				strcat(SendBuffer,"File_Explorer:");
				sendflag=1;
				break;
            }
            if(sendflag)
            {
                sendflag=0;
				SendSize=strlen(SendBuffer);
                if(CC.HTTP_Proxy)
                {
                    //通过HTTP隧道，需要先发送HTTP头
                    if (Send_HTTP_Header(CC.soc,SendSize))
                    {
                        memset(&CC,NULL,sizeof(ControlClient));
                        CC.soc=INVALID_SOCKET;
                        continue;
                    }
                }
				rc4_crypt((unsigned char *)S_box,(unsigned char *)SendBuffer,SendSize);   //加密
                if(send(CC.soc,SendBuffer,SendSize,0)<=0)
                {
                    //printf("客户端断开.");
                    CleanupClientConnection(CC.soc);
                }
            }
        }
        else
        {
            Sleep(1);
            continue;
        }
        memset(&CC,NULL,sizeof(ControlClient));
        CC.soc=INVALID_SOCKET;
    }
    return 0;
}