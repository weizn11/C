#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <conio.h>
#include <iphlpapi.h>

#define MAX_SIZE 1500
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

/*
定义
#define MAX_ADAPTER_NAME_LENGTH 256
#define MAX_ADAPTER_DESCRIPTION_LENGTH 128
#define MAX_ADAPTER_ADDRESS_LENGTH 8
typedef struct _IP_ADAPTER_INFO
{
struct _IP_ADAPTER_INFO* Next;//指向链表中下一个适配器信息的指针
DWORD ComboIndex;//预留值
char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];//适配器名称
char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];//适配器描述
UINT AddressLength;//适配器硬件地址以字节计算的长度
BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];//硬件地址以BYTE数组所表示
DWORD Index;//适配器索引
UINT Type;//适配器类型
UINT DhcpEnabled;//指定这个适配器是否开启
DHCP    PIP_ADDR_STRING CurrentIpAddress;//预留值
IP_ADDR_STRING IpAddressList;//该适配器的IPv4地址链表
IP_ADDR_STRING GatewayList;//该适配器的网关IPv4地址链表
IP_ADDR_STRING DhcpServer;//该适配器的DHCP服务器的IPv4 地址链表
BOOL HaveWins;
IP_ADDR_STRING PrimaryWinsServer;
IP_ADDR_STRING SecondaryWinsServer;
time_t LeaseObtained;
time_t LeaseExpires;
} IP_ADAPTER_INFO,*PIP_ADAPTER_INFO;

*/

typedef struct ipheader
{
    unsigned char ip_hl:4;				/*header length(报头长度）*/
    unsigned char ip_v:4;				/*version(版本)*/
    unsigned char ip_tos;				/*type os service服务类型*/
    unsigned short int ip_len;			/*total length (总长度)*/
    unsigned short int ip_id;			/*identification (标识符)*/
    unsigned short int ip_off;			/*fragment offset field(段移位域)*/
    unsigned char ip_ttl;				/*time to live (生存时间)*/
    unsigned char ip_p;					/*protocol(协议)*/
    unsigned short int ip_sum;			/*checksum(校验和)*/
    unsigned int ip_src;				/*source address(源地址)*/
    unsigned int ip_dst;				/*destination address(目的地址)*/
} IP;									/* total ip header length: 20 bytes (=160 bits) */

typedef struct tcpheader
{
    unsigned short int sport;			/*source port (源端口号)*/
    unsigned short int dport;			/*destination port(目的端口号)*/
    unsigned int th_seq;				/*sequence number(包的序列号)*/
    unsigned int th_ack;				/*acknowledgement number(确认应答号)*/
    unsigned char th_x:4;				/*unused(未使用)*/
    unsigned char th_off:4;				/*data offset(数据偏移量)*/
    unsigned char Flags;				/*标志全*/
    unsigned short int th_win;			/*windows(窗口)*/
    unsigned short int th_sum;			/*checksum(校验和)*/
    unsigned short int th_urp;			/*urgent pointer(紧急指针)*/
} TCP;

typedef struct udphdr
{
    unsigned short sport;				/*source port(源端口号)*/
    unsigned short dport;				/*destination port(目的端口号)*/
    unsigned short len;					/*udp length(udp长度)*/
    unsigned short cksum;				/*udp checksum(udp校验和)*/
} UDP;

char *Choose_Adapter()
{
    static char IP_Address[20];
    int Adapters_Amount=0;
    int Adapter;
    IP_ADAPTER_INFO AdapterInfo[16];     //定义存储网卡信息的结构数组
    DWORD ArrayLength=sizeof(AdapterInfo);                   //缓冲区长度

    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return NULL;
    PIP_ADAPTER_INFO PAdapterInfo = AdapterInfo;    //IP_ADAPTER_INFO结构体指针
    do
    {
        Adapters_Amount++;
        printf("-------------------------------------------------------\n%d.\n\n",Adapters_Amount);
        printf("网卡名:%s\n",PAdapterInfo->AdapterName);
        printf("网卡描述:%s\n",PAdapterInfo->Description);
        printf("IP地址:%s\n",PAdapterInfo->IpAddressList.IpAddress.String);
        printf("子网掩码:%s\n",PAdapterInfo->IpAddressList.IpMask.String);
        printf("网关:%s\n",PAdapterInfo->GatewayList.IpAddress.String);
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);
    printf("-------------------------------------------------------\n\n");

    printf("请选择一个网卡:");
    while(true)
    {
        fflush(stdin);
        if(scanf("%d",&Adapter)==1 && Adapter<=Adapters_Amount && Adapter>0) break;
        printf("输入有误,请重新选择:");
    }
    memset(IP_Address,NULL,sizeof(IP_Address));
    strcat(IP_Address,AdapterInfo[Adapter-1].IpAddressList.IpAddress.String);

    return IP_Address;
}

bool filter(IP *PIP,TCP *PTCP,UDP *PUDP,char *Sniff_IP,int Sniff_Port)
{
    if(strcmpi(Sniff_IP,"0"))
    {
        //嗅探指定的IP
        if(!strcmpi(Sniff_IP,inet_ntoa(*(in_addr *)&PIP->ip_src)) || !strcmpi(Sniff_IP,inet_ntoa(*(in_addr *)&PIP->ip_dst)))
        {
            if(Sniff_Port)
            {
                //嗅探指定的端口
                if(PTCP)
                {
                    if((int)ntohs(PTCP->sport)==Sniff_Port || (int)ntohs(PTCP->dport)==Sniff_Port)
                    {
                        return true;
                    }
                }
                else if(PUDP)
                {
                    if(ntohs(PUDP->sport)==Sniff_Port || ntohs(PUDP->dport)==Sniff_Port)
                    {
                        return true;
                    }
                }
            }
            else
                return true;
        }
    }
    else
    {
        if(Sniff_Port!=0)
        {
            //嗅探指定的端口
            if(PTCP)
            {
                if(ntohs(PTCP->sport)==Sniff_Port || ntohs(PTCP->dport)==Sniff_Port)
                {
                    return true;
                }
            }
            else if(PUDP)
            {
                if(ntohs(PUDP->sport)==Sniff_Port || ntohs(PUDP->dport)==Sniff_Port)
                {
                    return true;
                }
            }
        }
        else
            return true;
    }
    return false;
}

bool Sniffer(char *IP_Address,int Proto,char *Sniff_IP,int Sniff_Port)
{
    FILE *file=NULL;
    SOCKET soc=INVALID_SOCKET;
    struct sockaddr_in saddr;
    WSADATA wsa;
    char RecvBuff[MAX_SIZE];
    unsigned long SIO_Parameter=1;

    IP *PIP=(IP *)RecvBuff;         //指向IP结构体的指针
    TCP *PTCP=(TCP *)(RecvBuff+sizeof(IP));
    UDP *PUDP=(UDP *)(RecvBuff+sizeof(IP));

    memset(&saddr,NULL,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_addr.S_un.S_addr=inet_addr(IP_Address);
    saddr.sin_port=htons(0);

    if((file=fopen("sniffer.txt","wt"))==NULL)
    {
        printf("创建文件失败。\n");
        return false;
    }

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("初始化失败。\n");
        return false;
    }

    if((soc=socket(AF_INET,SOCK_RAW,IPPROTO_IP))==INVALID_SOCKET) //创建原始套接字
    {
        printf("创建套接字失败。\n");
        WSACleanup();
        return false;
    }

    if(bind(soc,(struct sockaddr *)&saddr,sizeof(saddr))!=0)
    {
        printf("绑定地址失败。\n");
        closesocket(soc);
        WSACleanup();
        return false;
    }

    ioctlsocket(soc,SIO_RCVALL,&SIO_Parameter);  //控制套接口模式，接收所有数据包。

    printf("开始嗅探...\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    while(true)
    {
        memset(RecvBuff,NULL,sizeof(RecvBuff));
        recv(soc,RecvBuff,sizeof(RecvBuff),0);
        if(PIP->ip_p==IPPROTO_TCP && (Proto==0 || Proto==1))
        {
            if(filter(PIP,PTCP,NULL,Sniff_IP,Sniff_Port))
            {
                printf("协议:TCP\n");
                printf("来源:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PTCP->sport));
                printf("目标:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PTCP->dport));
                puts("数据包内容:\n\n\n");
                printf("%s\n",RecvBuff+sizeof(IP)+sizeof(TCP));
                puts("\n\n--------------------------------------------------------------------\n");

                fprintf(file,"协议:TCP\n");
                fprintf(file,"来源:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PTCP->sport));
                fprintf(file,"目标:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PTCP->dport));
                fputs("数据包内容:\n\n\n",file);
                fprintf(file,"%s\n",RecvBuff+sizeof(IP)+sizeof(TCP));
                fputs("\n\n--------------------------------------------------------------------\n",file);
                fflush(file);
            }
        }
        else if(PIP->ip_p==IPPROTO_UDP && (Proto==0 || Proto==2))
        {
            if(filter(PIP,NULL,PUDP,Sniff_IP,Sniff_Port))
            {
                printf("协议:UDP\n");
                printf("来源:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PUDP->sport));
                printf("目标:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PUDP->dport));
                puts("数据包内容:\n\n\n");
                printf("%s\n",RecvBuff+sizeof(IP)+sizeof(UDP));
                puts("\n\n--------------------------------------------------------------------\n");

                fprintf(file,"协议:UDP\n");
                fprintf(file,"来源:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PUDP->sport));
                fprintf(file,"目标:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PUDP->dport));
                fputs("数据包内容:\n\n\n",file);
                fprintf(file,"%s\n",RecvBuff+sizeof(IP)+sizeof(UDP));
                fputs("\n\n--------------------------------------------------------------------\n",file);
                fflush(file);
            }
        }
        else
            continue;
    }
}

int main(int argc,char *argv[])
{
    system("color b");
    char *IP_Address=NULL;
    char Sniff_IP[20];
    int Sniff_Port;
    int Proto;

    if((IP_Address=Choose_Adapter())==NULL)
    {
        printf("获取网卡信息失败。\n");
        getch();
        return -1;
    }

    printf("请选择嗅探的协议(0.TCP & UDP; 1.TCP; 2.UDP):\n");
again1:
    fflush(stdin);
    if(scanf("%d",&Proto)!=1 || Proto<0 || Proto>2)
    {
        printf("输入有误，请重新选择:");
        goto again1;
    }
    memset(Sniff_IP,NULL,sizeof(Sniff_IP));
    printf("请输入所需嗅探的IP(\"0\"为嗅探所有IP):\n");
    fflush(stdin);
    gets(Sniff_IP);
    printf("请输入所需嗅探的端口(\"0\"为嗅探所有端口):\n");
again2:
    fflush(stdin);
    if(scanf("%d",&Sniff_Port)!=1)
    {
        printf("输入有误,请重新输入:");
        goto again2;
    }

    if(!Sniffer(IP_Address,Proto,Sniff_IP,Sniff_Port))
    {
        printf("启动嗅探失败。\n");
        getch();
        return -1;
    }
    return 0;
}












