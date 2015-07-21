#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

int Func1_Thread_Count=0,Func2_Thread_Count=0,Func3_Thread_Count=0;
CRITICAL_SECTION CS_FUNC_1,CS_COUNT_1;
CRITICAL_SECTION CS_FUNC_2,CS_COUNT_2;
CRITICAL_SECTION CS_FUNC_3,CS_COUNT_3;

typedef struct
{
    char TargetHost[50];
    int NowPort;
    FILE *file;
} FUNC1_PARA;

typedef struct
{
    char TargetHost[50];
    int TargetPort;
    FILE *file;
} FUNC2_PARA;

typedef struct
{
    char TargetHost[50];
    int NowPort;
    FILE *file;
} FUNC3_PARA;

int func1_print(int open_port,int now_port,FILE *file)
{
    printf("                                         \r");
    if(open_port>0)
    {
        printf("Port:%d  ------->  Open\n",open_port);
        fprintf(file,"Port:%d  ------->  Open\n",open_port);
        fflush(file);
    }
    else if(now_port>0)
        printf("Check Port:%d  Thread:%d\r",now_port,Func1_Thread_Count);

    return 0;
}

int func2_print(char *target_host,int sign,FILE *file)
{
    printf("                                                                               \r");
    if(sign)
    {
        printf("%s\n",target_host);
        fprintf(file,"%s\n",target_host);
        fflush(file);
    }
    else
        printf("Scan Host:%s   Thread:%d\r",target_host,Func2_Thread_Count);

    return 0;
}

int func3_print(char *target_host,int open_port,int now_port,FILE *file)
{
    static char LastIP[50]= {0};
    printf("                                                               \r");
    if(now_port>0)
        printf("Scan Host:%s  Check Port:%d   Thread:%d\r",target_host,now_port,Func3_Thread_Count);
    else
    {
        if(strcmp(LastIP,target_host)!=0)
        {
            memset(LastIP,NULL,sizeof(LastIP));
            strcat(LastIP,target_host);
            printf("-----------------------------------------------------\n");
            fprintf(file,"-----------------------------------------------------\n");
        }
        printf("Host:%s   Port:%d -------> Open\n",target_host,open_port);
        fprintf(file,"Host:%s   Port:%d -------> Open\n",target_host,open_port);
        fflush(file);
    }

    return 0;
}

int check_port(char *target_ip,int target_port)
{
    SOCKET soc=INVALID_SOCKET;
    struct sockaddr_in addr;
    struct timeval timeout;
    unsigned long socpara=1;
    fd_set fd_write;

    memset(&addr,NULL,sizeof(struct sockaddr_in));
    memset(&timeout,NULL,sizeof(struct timeval));

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(target_ip);
    addr.sin_port=htons(target_port);
    timeout.tv_sec=10;

    if((soc=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
        return -1;

    ioctlsocket(soc,FIONBIO,&socpara);       //设置为非阻塞模式
    if(connect(soc,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))==0)
    {
        closesocket(soc);
        return 0;
    }

    FD_ZERO(&fd_write);
    FD_SET(soc,&fd_write);
    if(select(-1,NULL,&fd_write,NULL,&timeout)>0)
    {
        closesocket(soc);
        return 0;
    }

    closesocket(soc);
    return -1;
}

DWORD WINAPI func1_thread(LPVOID Parameter)
{
    FUNC1_PARA *para=(FUNC1_PARA *)Parameter;

    EnterCriticalSection(&CS_COUNT_1);
    Func1_Thread_Count++;
    LeaveCriticalSection(&CS_COUNT_1);

    if(check_port(para->TargetHost,para->NowPort)!=0)
    {
        EnterCriticalSection(&CS_COUNT_1);
        Func1_Thread_Count--;
        LeaveCriticalSection(&CS_COUNT_1);
        return -1;
    }

    EnterCriticalSection(&CS_FUNC_1);
    func1_print(para->NowPort,-1,para->file);
    LeaveCriticalSection(&CS_FUNC_1);

    EnterCriticalSection(&CS_COUNT_1);
    Func1_Thread_Count--;
    LeaveCriticalSection(&CS_COUNT_1);

    free(para);

    return 0;
}

int func_1()
{
    int StartPort,EndPort;
    FUNC1_PARA *para=NULL;
    char TargetIP[50];
    FILE *file=NULL;

    memset(TargetIP,NULL,sizeof(TargetIP));

    if((file=fopen("results.txt","wt"))==NULL)
    {
        printf("Create result file failed!\n");
        getch();
        return -1;
    }

    printf("Target IP:");
    fflush(stdin);
    scanf("%s",TargetIP);
    printf("Start Port:");
    fflush(stdin);
    if(scanf("%d",&StartPort)!=1)
        return -1;
    printf("End Port:");
    fflush(stdin);
    if(scanf("%d",&EndPort)!=1)
        return -1;

    puts("\n");
    if(StartPort<1 || StartPort>EndPort || EndPort>65535)
        return -1;
    for(; StartPort<=EndPort; StartPort++)
    {
        if((para=(FUNC1_PARA *)malloc(sizeof(FUNC1_PARA)))==NULL)
            break;
        memset(para,NULL,sizeof(FUNC1_PARA));
        para->NowPort=StartPort;
        strcat(para->TargetHost,TargetIP);
        para->file=file;
        EnterCriticalSection(&CS_FUNC_1);
        func1_print(-1,StartPort,NULL);
        LeaveCriticalSection(&CS_FUNC_1);
        CloseHandle(CreateThread(NULL,0,func1_thread,(LPVOID)para,0,NULL));
        Sleep(10);
        para=NULL;
        while(Func1_Thread_Count>=1000) Sleep(100);
    }
    EnterCriticalSection(&CS_FUNC_1);
    printf("                                                       \r");
    printf("Wait for all threads to exit.\r");
    LeaveCriticalSection(&CS_FUNC_1);
    while(Func1_Thread_Count!=0) Sleep(500);
    fclose(file);
    EnterCriticalSection(&CS_FUNC_1);
    printf("                                                       \r");
    puts("\nAll Done!");
    LeaveCriticalSection(&CS_FUNC_1);

    return 0;
}

DWORD WINAPI func2_thread(LPVOID Parameter)
{
    FUNC2_PARA *para=(FUNC2_PARA *)Parameter;
    EnterCriticalSection(&CS_COUNT_2);
    Func2_Thread_Count++;
    LeaveCriticalSection(&CS_COUNT_2);

    if(check_port(para->TargetHost,para->TargetPort)==0)
    {
        EnterCriticalSection(&CS_FUNC_2);
        func2_print(para->TargetHost,1,para->file);
        LeaveCriticalSection(&CS_FUNC_2);
    }

    EnterCriticalSection(&CS_COUNT_2);
    Func2_Thread_Count--;
    LeaveCriticalSection(&CS_COUNT_2);

    free(para);

    return 0;
}

int func_2()
{
    char StartIP[50],EndIP[50];
    char *pStart=NULL,*pEnd=NULL,temp[50];
    int TargetPort,region[2][4]= {0},i=0,j=0;
    FUNC2_PARA *para=NULL;
    FILE *file=NULL;

    memset(StartIP,NULL,sizeof(StartIP));
    memset(EndIP,NULL,sizeof(EndIP));

    if((file=fopen("results.txt","wt"))==NULL)
    {
        printf("Create result file failed!\n");
        getch();
        return -1;
    }

    printf("Start IP:");
    fflush(stdin);
    scanf("%s",StartIP);
    printf("End IP:");
    fflush(stdin);
    scanf("%s",EndIP);
    printf("Port:");
    fflush(stdin);
    if(scanf("%d",&TargetPort)!=1)
        return -1;

    puts("\n");
    strcat(StartIP,".");
    strcat(EndIP,".");
    for(pStart=StartIP,i=0,j=0; 1; i++)
    {
        memset(temp,NULL,sizeof(temp));
        pEnd=strchr(pStart,'.');
        if(pEnd==NULL)
        {
            printf("Input is wrong!\n");
            getch();
            return -1;
        }
        memcpy(temp,pStart,pEnd-pStart);
        region[j][i]=atoi(temp);
        if(i==3)
        {
            if(j==1) break;
            pStart=EndIP;
            j++;
            i=-1;
        }
        else
            pStart=pEnd+1;
    }

    while(region[1][0]>region[0][0] || region[1][1]>region[0][1] || region[1][2]>region[0][2] || region[1][3]>=region[0][3])
    {
        para=(FUNC2_PARA *)malloc(sizeof(FUNC2_PARA));
        if(para==NULL)
        {
            printf("\nmalloc() error!\n");
            break;
        }
        memset(para,NULL,sizeof(FUNC2_PARA));
        sprintf(para->TargetHost,"%d.%d.%d.%d",region[0][0],region[0][1],region[0][2],region[0][3]);
        para->TargetPort=TargetPort;
        para->file=file;
        EnterCriticalSection(&CS_FUNC_2);
        func2_print(para->TargetHost,0,NULL);
        LeaveCriticalSection(&CS_FUNC_2);
        CloseHandle(CreateThread(NULL,0,func2_thread,(LPVOID)para,0,NULL));
        Sleep(10);
        region[0][3]++;
        if(region[0][3]>255)
        {
            region[0][3]=1;
            region[0][2]++;
            if(region[0][2]>255)
            {
                region[0][2]=0;
                region[0][1]++;
                if(region[0][1]>255)
                {
                    region[0][1]=0;
                    region[0][0]++;
                    if(region[0][0]>255) break;
                }
            }
        }
        para=NULL;
        while(Func2_Thread_Count>=1000) Sleep(500);
    }
    EnterCriticalSection(&CS_FUNC_2);
    printf("                                                       \r");
    printf("Wait for all threads to exit.\r");
    LeaveCriticalSection(&CS_FUNC_2);
    while(Func2_Thread_Count!=0) Sleep(500);
    fclose(file);
    EnterCriticalSection(&CS_FUNC_2);
    printf("                                                       \r");
    puts("\nAll Done!");
    LeaveCriticalSection(&CS_FUNC_2);

    return 0;
}

DWORD WINAPI func3_thread(LPVOID Parameter)
{
    FUNC3_PARA *para=(FUNC3_PARA *)Parameter;

    EnterCriticalSection(&CS_COUNT_3);
    Func3_Thread_Count++;
    LeaveCriticalSection(&CS_COUNT_3);

    if(check_port(para->TargetHost,para->NowPort)!=0)
    {
        EnterCriticalSection(&CS_COUNT_3);
        Func3_Thread_Count--;
        LeaveCriticalSection(&CS_COUNT_3);
        return -1;
    }

    EnterCriticalSection(&CS_FUNC_3);
    func3_print(para->TargetHost,para->NowPort,-1,para->file);
    LeaveCriticalSection(&CS_FUNC_3);

    EnterCriticalSection(&CS_COUNT_3);
    Func3_Thread_Count--;
    LeaveCriticalSection(&CS_COUNT_3);

    free(para);

    return 0;
}

int func_3()
{
    char StartIP[50],EndIP[50],temp[50];
    char *pStart=NULL,*pEnd=NULL;
    int StartPort,NowPort,EndPort,region[2][4]= {0},i=0,j=0;
    FUNC3_PARA *para=NULL;
    FILE *file=NULL;

    memset(StartIP,NULL,sizeof(StartIP));
    memset(EndIP,NULL,sizeof(EndIP));

    if((file=fopen("results.txt","wt"))==NULL)
    {
        printf("Create result file failed!\n");
        getch();
        return -1;
    }

    printf("Start IP:");
    fflush(stdin);
    scanf("%s",StartIP);
    printf("End IP:");
    fflush(stdin);
    scanf("%s",EndIP);
    printf("Start Port:");
    fflush(stdin);
    if(scanf("%d",&StartPort)!=1) return -1;
    printf("End Port:");
    fflush(stdin);
    if(scanf("%d",&EndPort)!=1) return -1;
    if(StartPort<1 || StartPort>EndPort || EndPort>65535)
        return -1;

    puts("");
    strcat(StartIP,".");
    strcat(EndIP,".");
    for(pStart=StartIP,i=0,j=0; 1; i++)
    {
        memset(temp,NULL,sizeof(temp));
        pEnd=strchr(pStart,'.');
        if(pEnd==NULL)
        {
            printf("Input is wrong!\n");
            getch();
            return -1;
        }
        memcpy(temp,pStart,pEnd-pStart);
        region[j][i]=atoi(temp);
        if(i==3)
        {
            if(j==1) break;
            pStart=EndIP;
            j++;
            i=-1;
        }
        else
            pStart=pEnd+1;
    }

    while(region[1][0]>region[0][0] || region[1][1]>region[0][1] || region[1][2]>region[0][2] || region[1][3]>=region[0][3])
    {
        for(NowPort=StartPort; NowPort<=EndPort; NowPort++)
        {
            para=(FUNC3_PARA *)malloc(sizeof(FUNC3_PARA));
            if(para==NULL)
            {
                printf("\nmalloc() error!\n");
                break;
            }
            memset(para,NULL,sizeof(FUNC3_PARA));
            sprintf(para->TargetHost,"%d.%d.%d.%d",region[0][0],region[0][1],region[0][2],region[0][3]);
            para->NowPort=NowPort;
            para->file=file;
            EnterCriticalSection(&CS_FUNC_3);
            func3_print(para->TargetHost,-1,para->NowPort,NULL);
            LeaveCriticalSection(&CS_FUNC_3);
            CloseHandle(CreateThread(NULL,0,func3_thread,(LPVOID)para,0,NULL));
            Sleep(10);
            para=NULL;
            while(Func3_Thread_Count>=1000) Sleep(100);
        }
        while(Func3_Thread_Count>0) Sleep(500);

        region[0][3]++;
        if(region[0][3]>255)
        {
            region[0][3]=1;
            region[0][2]++;
            if(region[0][2]>255)
            {
                region[0][2]=0;
                region[0][1]++;
                if(region[0][1]>255)
                {
                    region[0][1]=0;
                    region[0][0]++;
                    if(region[0][0]>255) break;
                }
            }
        }
    }
    EnterCriticalSection(&CS_FUNC_3);
    printf("                                                       \r");
    printf("Wait for all threads to exit.\r");
    LeaveCriticalSection(&CS_FUNC_3);
    while(Func3_Thread_Count!=0) Sleep(500);
    fclose(file);
    EnterCriticalSection(&CS_FUNC_3);
    printf("                                                       \r");
    puts("\nAll Done!");
    LeaveCriticalSection(&CS_FUNC_3);

    return 0;
}

int main(int argc,char *argv[])
{
    char choose;
    WSADATA wsa;

    memset(&wsa,NULL,sizeof(WSADATA));

    InitializeCriticalSection(&CS_FUNC_1);
    InitializeCriticalSection(&CS_COUNT_1);
    InitializeCriticalSection(&CS_FUNC_2);
    InitializeCriticalSection(&CS_COUNT_2);
    InitializeCriticalSection(&CS_FUNC_3);
    InitializeCriticalSection(&CS_COUNT_3);

    system("color a");
    SetConsoleTitle("Port Scanner  --By:Wayne");

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("WSAStartup() error!\n");
        getch();
        return -1;
    }

again:
    system("cls");
    printf("Please select an option:\n1.You can scan all ports on the target host.\n"
           "2.You can scan a port of some hosts.\n3.You can scan all ports of some hosts.\n");

    do
    {
        fflush(stdin);
        choose=getch();
    }
    while(choose<'1' || choose>'3');
    system("cls");
    switch(choose)
    {
    case '1':
        func_1();
        break;
    case '2':
        func_2();
        break;
    case '3':
        func_3();
        break;
    }
    getch();
    goto again;

    return 0;
}













