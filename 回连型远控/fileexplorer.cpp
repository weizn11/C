#include "fileexplorer.h"

void file_explorer_tree_selection(GtkWidget *view,GtkTreePath *path,GtkTreeViewColumn *col,gpointer Parameter)
{
    Explorer_Struct *es=(Explorer_Struct *)Parameter;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *explorer_selection;

    EnterCriticalSection(&es->cs);
    if(view==es->drives_tree_view)
    {
        explorer_selection=es->drive_selection;
        es->buffer_type=1;
    }
    else if(view==es->folders_tree_view)
    {
        explorer_selection=es->folder_selection;
        es->buffer_type=2;
    }
    else
        return;
    if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(explorer_selection),&model,&iter))
    {
        LeaveCriticalSection(&es->cs);
        return;
    }
    gtk_tree_model_get(model,&iter,0,&es->buffer,-1);
    es->refresh_flag=1;
    LeaveCriticalSection(&es->cs);
    //MessageBox(NULL,es->buffer,"info",MB_OK);

    return;
}

void file_explorer(gpointer Parameter)
{
    //新线程(必须与主线程(gtk_main())进行互斥操作)
    Explorer_Struct *es=(Explorer_Struct *)Parameter;
    int i=0,n=0,write_len=0,sur_len=0,recv_size=0,SendSize=0;;
    for(i=0; File_Explorer_Socket==INVALID_SOCKET && i<10000; i++) Sleep(1);
    if(File_Explorer_Socket==INVALID_SOCKET) g_thread_exit(NULL);
    es->soc=File_Explorer_Socket;
    File_Explorer_Socket=INVALID_SOCKET;
    SOCKET soc=es->soc;
    char RecvBuffer[MAX_SIZE+1],SendBuffer[MAX_SIZE];
    char *buffer=NULL,*str=NULL,*ptmp=NULL,*pStart=NULL,*http_buffer=NULL,*recv_buffer=NULL;
    char path_label[MAX_SIZE],oldfile[MAX_SIZE],newfile[MAX_SIZE];
    FileExplorerStruct *FES=(FileExplorerStruct *)RecvBuffer;
    struct timeval WaitTimeOut;
    GtkListStore *temp_store=NULL;
    GtkWidget *temp_view=NULL;
    fd_set readfd,writefd;
    int send_flag=0,ret=0,recv_buffer_size=0;

    memset(es->path,NULL,MAX_SIZE);
    memset(path_label,NULL,sizeof(path_label));
    memset(oldfile,NULL,sizeof(oldfile));
    memset(newfile,NULL,sizeof(newfile));

    if(!IODataArray[es->index]->Client.HTTP_Proxy)
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
        if(es->exit_flag==1)
        {
            //printf("file_explorer线程退出\n");
			if(recv_buffer!=NULL)
				free(recv_buffer);
            if(buffer!=NULL)
                free(buffer);
			if(http_buffer!=NULL)
				free(http_buffer);
            DeleteCriticalSection(&es->cs);
            free(es);
            g_thread_exit(NULL);
        }
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);
        FD_SET(soc,&readfd);
        FD_SET(soc,&writefd);
        if(select(-1,&readfd,&writefd,NULL,&WaitTimeOut)<1)
            continue;
        if(FD_ISSET(soc,&readfd))
        {
            do
            {
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                if((recv_size=recv(soc,RecvBuffer,sizeof(RecvBuffer)-1,0))<=0)
                {
                    closesocket(soc);
                    if(buffer!=NULL)
                        free(buffer);
					if(http_buffer!=NULL)
					    free(http_buffer);
                    g_thread_exit(NULL);
                }
                //printf("接收到:%d\n",recv_size);
                if(IODataArray[es->index]->Client.HTTP_Proxy)
                {
                    //过滤HTTP_Header
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

                rc4_crypt(S_box,(unsigned char *)RecvBuffer,sizeof(RecvBuffer)-1);   //解密
                //printf("recv:%s\n",FES->content);
                if(buffer==NULL)
                {
                    if((buffer=(char *)malloc(1453*sizeof(char)))==NULL)
                    {
                        MessageBox(NULL,"malloc error!","Error",MB_OK);
                        exit(-1);
                    }
                    n=1;
                }
                else
                {
                    n++;
                    if((buffer=(char *)realloc(buffer,1452*sizeof(char)*n+1))==NULL)
                    {
                        MessageBox(NULL,"realloc error","Error",MB_OK);
                        exit(-1);
                    }
                }
                memset(buffer+(n-1)*1452,NULL,1453);
                memcpy(buffer+(n-1)*1452,FES->content,1452);
                if(FES->end_flag==0)
                {
                    //数据已接收完成
                    EnterCriticalSection(&es->cs);
                    switch (FES->type_flag)
                    {
                    case 0:
                        memset(es->path,NULL,sizeof(es->path));
                        strcat(es->path,buffer);
                        if(es->path[strlen(es->path)-1]!='\\')
                            strcat(es->path,"\\");
                        es->refresh_flag=1;
                        break;
                    case 1:
                        temp_store=es->drives_list_store;
                        temp_view=es->drives_tree_view;
                        break;
                    case 2:
                        temp_store=es->folders_list_store;
                        temp_view=es->folders_tree_view;
                        break;
                    case 3:
                        temp_store=es->files_list_store;
                        temp_view=es->files_tree_view;
                        break;
                    }
                    LeaveCriticalSection(&es->cs);
                    if(FES->type_flag!=0)
                    {
                        str=strtok(buffer,",");
                        while(str!=NULL)
                        {
                            //MessageBox(NULL,str,"1",MB_OK);
                            gdk_threads_enter();
                            add_explorer_list(temp_store,str);
                            gdk_threads_leave();
                            str=strtok(NULL,",");
                        }
                    }
                    free(buffer);
                    buffer=NULL;
                    n=0;
                }
                if(IODataArray[es->index]->Client.HTTP_Proxy)
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
                }
            }
            while(FES->end_flag!=0);
        }
        EnterCriticalSection(&es->cs);
        if(es->refresh_flag)
        {
            gdk_threads_enter();
            remove_explorer_item(es->folders_tree_view,es->folders_list_store);
            remove_explorer_item(es->files_tree_view,es->files_list_store);
            gdk_threads_leave();
            if(es->buffer!=NULL && strlen(es->buffer))
            {
                if(es->buffer_type==1)
                {
                    memset(es->path,NULL,sizeof(es->path));
                    strcat(es->path,es->buffer);
                }
                else if(es->buffer_type==2)
                {
                    if(!strcmp(es->buffer,"."))
                    {
                        memset(&es->path[3],NULL,sizeof(es->path)-3);
                    }
                    else if(!strcmp(es->buffer,".."))
                    {
                        for(ptmp=&(es->path[strlen(es->path)-2]); *ptmp!='\\'; ptmp--);
                        ptmp++;
                        memset(ptmp,NULL,strlen(ptmp));
                    }
                    else
                    {
                        if(es->path[strlen(es->path)-1]!='\\')
                            strcat(es->path,"\\");
                        EnterCriticalSection(&CRI_UTA);
                        strcat(es->path,UTF8ToANSI(es->buffer));
                        LeaveCriticalSection(&CRI_UTA);
                    }
                    if(es->path[strlen(es->path)-1]!='\\')
                        strcat(es->path,"\\");
                }
                g_free(es->buffer);
                es->buffer=NULL;
            }
            memset(path_label,NULL,sizeof(path_label));
            sprintf(path_label,"%s",es->path);
            gdk_threads_enter();
            EnterCriticalSection(&CRI_ATU);
            gtk_label_set_text(GTK_LABEL(es->path_label),ANSIToUTF8(path_label));
            LeaveCriticalSection(&CRI_ATU);
            gdk_threads_leave();
            memset(SendBuffer,NULL,sizeof(SendBuffer));
            strcat(SendBuffer,es->path);
            es->refresh_flag=0;
            if(strlen(SendBuffer))
                send_flag=1;
        }
        else if(es->delete_flag)
        {
            memset(SendBuffer,NULL,sizeof(SendBuffer));
            EnterCriticalSection(&CRI_UTA);
            sprintf(SendBuffer,"delete|%s%s",es->path,UTF8ToANSI(es->filename));
            LeaveCriticalSection(&CRI_UTA);
            g_free(es->filename);
            es->filename=NULL;
            es->refresh_flag=1;
            es->delete_flag=0;
            send_flag=1;
        }
        LeaveCriticalSection(&es->cs);
        if(send_flag)
        {
            Sleep(500);
            SendSize=strlen(SendBuffer);
            if(FD_ISSET(soc,&writefd))
            {
                //printf("%s\n",SendBuffer);
                rc4_crypt(S_box,(unsigned char *)SendBuffer,SendSize);
                if(send(soc,SendBuffer,SendSize,0)<=0)
                    g_thread_exit(NULL);
                send_flag=0;
            }
        }
        else
            Sleep(10);
    }

    return;
}