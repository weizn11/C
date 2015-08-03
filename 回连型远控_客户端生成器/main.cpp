#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <gtk/gtk.h>

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") 

typedef struct  
{
	GtkWidget *Server_IP_entry;
	GtkWidget *Server_Port_entry;
	GtkWidget *Proxy_IP_entry;
	GtkWidget *Proxy_Port_entry;
	GtkWidget *Proxy_Username_entry;
	GtkWidget *Proxy_Password_entry;
}ClientConfigStruct;

int release_client()
{
	//释放资源文件
	HANDLE hFile=NULL,hRes=NULL;
	HRSRC hRsrc=NULL;
	DWORD FileSize=0,BytesWritten=0;
	LPVOID lpFile=NULL;

	if((hRsrc=FindResource(NULL,"Client","EXE"))==NULL)
	{
		CloseHandle(hRsrc);
		return -1;
	}

	if((hRes=LoadResource(NULL,hRsrc))==NULL)
	{
		CloseHandle(hRsrc);
		CloseHandle(hRes);
		return -1;
	}

	if((lpFile=LockResource(hRes))==NULL)
	{
		CloseHandle(hRes);
		CloseHandle(hRsrc);
		return -1;
	}

	if((FileSize=SizeofResource(NULL,hRsrc))==0)
	{
		CloseHandle(hRes);
		CloseHandle(hRsrc);
		return -1;
	}

	if((hFile=CreateFile("Client.exe",GENERIC_ALL,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL))==NULL)
	{
		CloseHandle(hRes);
		CloseHandle(hRsrc);
		CloseHandle(hFile);
		return -1;
	}

	if(WriteFile(hFile,lpFile,FileSize,&BytesWritten,NULL)==0)
	{
		CloseHandle(hRes);
		CloseHandle(hRsrc);
		CloseHandle(hFile);
		return -1;
	}

	CloseHandle(hRes);
	CloseHandle(hRsrc);
	CloseHandle(hFile);

	return 0;
}

char *match_str(char *m_str,char *p_str,unsigned long m_str_len,unsigned long p_str_len)
{
	int i,j;

	for (i=0;i<m_str_len;i++)
	{
		for (j=0;m_str[i+j]==p_str[j];j++)
		{
			if((i+j+1)==m_str_len && (j+1)!=p_str_len)
				return NULL;
			if((j+1)==p_str_len)
				return &m_str[i+j];
		}
	}
	return NULL;
}

int create_client(GtkWidget *widget,gpointer Parameter)
{
	ClientConfigStruct *CCS=(ClientConfigStruct *)Parameter;
	FILE *file=NULL;
	unsigned long FileSize=0;
	char *FileBuffer=NULL,*strbuffer=NULL,*pStart=NULL,tmp=85;
	int i;

	if(release_client()!=0)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		return -1;
	}

	file=fopen("Client.exe","rb+");
	if(file==NULL)
	{
		MessageBox(NULL,"Open file failed!","ERROR",MB_OK);
	}
	fseek(file,0,SEEK_END);
	FileSize=ftell(file);
	fseek(file,0,SEEK_SET);
	FileBuffer=(char *)malloc(sizeof(char)*FileSize);
	if(FileBuffer==NULL)
		exit(-1);
	memset(FileBuffer,NULL,sizeof(char)*FileSize);
	fread(FileBuffer,sizeof(char),FileSize,file);

	/*-------------------修改客户端文件内容---------------------*/
	//修改服务端IP地址
	pStart=match_str(FileBuffer,"SERVER_IP:",FileSize,strlen("SERVER_IP:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Server_IP_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=strlen("SERVER_IP:"),i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	//修改服务端监听端口
	pStart=match_str(FileBuffer,"SERVER_PORT:",FileSize,strlen("SERVER_PORT:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Server_Port_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=strlen("SERVER_PORT:"),i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	//修改HTTP代理服务器IP
	pStart=match_str(FileBuffer,"PROXY_IP:",FileSize,strlen("PROXY_IP:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Proxy_IP_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=strlen("PROXY_IP:"),i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	//修改HTTP代理服务器连接端口
	pStart=match_str(FileBuffer,"PROXY_PORT:",FileSize,strlen("PROXY_PORT:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Proxy_Port_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=strlen("PROXY_PORT:"),i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	//修改代理服务器登陆账号
	pStart=match_str(FileBuffer,"PROXY_USERNAME:",FileSize,strlen("PROXY_USERNAME:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Proxy_Username_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=15,i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	//修改代理服务器登陆密码
	pStart=match_str(FileBuffer,"PROXY_PASSWORD:",FileSize,strlen("PROXY_PASSWORD:"));
	if(pStart==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	pStart++;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(CCS->Proxy_Password_entry));
	memcpy(pStart,strbuffer,strlen(strbuffer));
	for(pStart-=strlen("PROXY_PASSWORD:"),i=0;i<50;pStart++,i++)
        *pStart^=tmp;

	pStart=match_str(FileBuffer,"RC4_KEY:",FileSize,strlen("RC4_KEY:"));
	for(pStart-=strlen("RC4_KEY:"),i=0;i<256;pStart++,i++)
        *pStart^=tmp;

	/*-------------------保存修改的内容---------------------*/
	fclose(file);
	if((file=fopen("Client.exe","wb"))==NULL)
	{
		MessageBox(NULL,"Create client failed!","ERROR",MB_OK);
		fclose(file);
		free(FileBuffer);
		return -1;
	}
	fwrite(FileBuffer,sizeof(char),FileSize,file);

	fclose(file);
	free(FileBuffer);
	MessageBox(NULL,"Create client successfully!","Success",MB_OK);

	return 0;
}

GtkWidget *create_window(char *title,int border_width,int length,int width)
{
    GtkWidget *window;
	
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),title);
    gtk_container_set_border_width(GTK_CONTAINER(window),border_width);
    gtk_widget_set_size_request(GTK_WIDGET(window),length,width);
    gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
    gtk_window_set_position( GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);
	
	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_widget_destroy),window);
	
    return window;
}

void destroy_create_client_window(GtkWidget *widget,gpointer Parameter)
{
	free(Parameter);
	exit(-1);
	return;
}

int create_client_window()
{
	GtkWidget *window;
	GtkWidget *vbox,*hbox;
	GtkWidget *button;
	GtkWidget *label;
	ClientConfigStruct *CCS;
	
	CCS=(ClientConfigStruct *)malloc(sizeof(ClientConfigStruct));
	if(CCS==NULL)
		exit(-1);
	memset(CCS,NULL,sizeof(ClientConfigStruct));
	
	window=create_window("Create Client",10,320,270);
	vbox=gtk_vbox_new(TRUE,5);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("Server IP/Domain:       ");
	CCS->Server_IP_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Server_IP_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("Server Port:                ");
	CCS->Server_Port_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Server_Port_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("HTTP Proxy IP/Domain:");
	CCS->Proxy_IP_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Proxy_IP_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("HTTP Proxy Port:         ");
	CCS->Proxy_Port_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Proxy_Port_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("HTTP Proxy User:        ");
	CCS->Proxy_Username_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Proxy_Username_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	hbox=gtk_hbox_new(FALSE,2);
	label=gtk_label_new_with_mnemonic("HTTP Proxy Password: ");
	CCS->Proxy_Password_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),CCS->Proxy_Password_entry,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	
	button=gtk_button_new_with_label("Build");
	gtk_box_pack_start(GTK_BOX(vbox),button,TRUE,FALSE,2);
	
	gtk_container_add(GTK_CONTAINER(window),vbox);
	
	g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(destroy_create_client_window),CCS);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(create_client),CCS);
	gtk_widget_show_all(window);
	
	return 0;
}

int main(int argc,char *argv[])
{
	gtk_init (&argc, &argv);
	create_client_window();
	gtk_main();
	return 0;
}