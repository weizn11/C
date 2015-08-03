#ifndef _GUI_H_
#define _GUI_H_

#include "global.h"
#include "encoding.h"
#include "fileexplorer.h"

//函数原型
void shell_window(GtkWidget *widget,gpointer selection);
void upload_window(GtkWidget *widget,gpointer selection);
void download_window(GtkWidget *widget,gpointer selection);
void disconnect(GtkWidget *widget,gpointer selection);
/*---------------------------------*/

//全局变量
enum
{
    HostID_Column,
    HostName_Column,
    HostIP_Column,
    HostOS_Column,
    Notes_Column,
    MAX_Column
};

typedef struct
{
    int index;
    GtkWidget *text_view;
    SOCKET soc;
    GtkWidget *entry;
} shell_struct;

typedef struct  
{
	GtkWidget *pbar;
	GtkWidget *local_path_entry;
	GtkWidget *remote_path_entry;
	GtkWidget *button;
	int handler;
	SOCKET soc;
	int order;        //2:上传文件,3:下载文件
}TranStruct;

typedef struct
{
	int flag;
	SOCKET ClientSocket;
	GtkTreeSelection *selection;
	char local_path[MAX_SIZE];
	char remote_path[MAX_SIZE];
}TranStructParameter;

int TotalList=1;
GtkWidget *tree_view;
/*---------------------------------*/


#endif   //_GUI_H_
