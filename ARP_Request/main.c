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

typedef struct
{
    pcap_t *hpcap;                        //网卡描述字
    unsigned char myIP[4];                //本机网卡IP
    unsigned char srcMAC[6];              //源MAC
    unsigned char desMAC[6];              //目标MAC
    unsigned char desIP[4];               //目标IP
    char *Packet;                         //数据包指针
    struct pcap_pkthdr pkthdr;            //储存数据包大小
} PacketInfo;

BOOL GetAdapterMAC(char *ipbuff,char *macbuff)
{
    IP_ADAPTER_INFO AdapterInfo[16];  //定义存储网卡信息的结构数组
    DWORD ArrayLength=sizeof(AdapterInfo);  //缓冲区长度
    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return ERROR;
    PIP_ADAPTER_INFO PAdapterInfo=AdapterInfo;
    puts(ipbuff);
    do
    {
        if(!strcmp(ipbuff,PAdapterInfo->IpAddressList.IpAddress.String)) break;
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);

    memset(macbuff,NULL,7);
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
    if(pcap_findalldevs(&alldevs,errbuff)==-1)
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
    memset(ipbuff,NULL,16);
    a=p->addresses;
    strcat(ipbuff,iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
    pcap_freealldevs(alldevs);

    return TRUE;
}

void Fill_ARPPACKET(char *ARPPacket,int packetsize,char *desmac,char *desIP,char *srcmac,char *srcip,int op)
{
    unsigned long temp;
    DLCHEADER *DLCHeader=(DLCHEADER *)ARPPacket;
    ARPFRAME *ARPFrame=(ARPFRAME *)(ARPPacket+sizeof(DLCHEADER));

    memset(ARPPacket,NULL,packetsize);  //清空包内容

    //填充以太网目的地址
    if(op==1)
    {
        memset(DLCHeader->DesMAC,0xff,6);
        memset(ARPFrame->Targ_HW_Addr,NULL,6);
    }
    else
    {
        memcpy(DLCHeader->DesMAC,desmac,6);
        memcpy(ARPFrame->Targ_HW_Addr,DLCHeader->DesMAC,6);
    }

    //填充以太网源地址
    memcpy(DLCHeader->SrcMAC,srcmac,6);
    memcpy(ARPFrame->Send_HW_Addr,DLCHeader->SrcMAC,sizeof(DLCHeader->SrcMAC));
    //填充ARP请求包源IP
    temp=inet_addr(srcip);
    memcpy(ARPFrame->Send_Prot_Addr,(char *)&temp,4);
    //填充ARP请求包目标IP
    temp=inet_addr(desIP);
    memcpy(ARPFrame->Targ_Prot_Addr,(char *)&temp,4);

    DLCHeader->EtherType=htons((unsigned short)0x0806);    //0x0806表示ARP协议，0x0800表示IP协议
    ARPFrame->HW_Addr_Len=(unsigned char)6;
    ARPFrame->Prot_Addr_Len=(unsigned char)4;
    ARPFrame->HW_Type=htons((unsigned short)1);
    ARPFrame->Opcode=htons((unsigned short)op);   //01表示请求，02表示应答
    ARPFrame->Prot_Type=htons((unsigned short)0x0800);
}

DWORD WINAPI filter(PVOID Parameter)
{
    PacketInfo PI=*(PacketInfo *)Parameter;
    char *Packet=NULL;

    Packet=(char *)malloc(PI.pkthdr.caplen);     //申请储存数据包空间
    memcpy(Packet,PI.Packet,PI.pkthdr.caplen);

    ARPFRAME *ARPFrame;
    DLCHEADER *DLCHeader=(DLCHEADER *)Packet;

    char tempmac[6];
    memset(tempmac,0xff,6);

    if(ntohs(DLCHeader->EtherType)==0x0806)
    {
        ARPFrame=(ARPFRAME *)(Packet+sizeof(DLCHEADER));
        if(!strncmp(DLCHeader->DesMAC,tempmac,6) && !strncmp(ARPFrame->Send_Prot_Addr,PI.myIP,4))
        {
            system("color b");
            int i;
            puts("");
            for(i=0; i<6; i++)
                printf("%02x ",ARPFrame->Send_HW_Addr[i]);
            puts("");
            printf("数据包大小:%d\n",PI.pkthdr.caplen);
            printf("数据包内容:\n");
            for(i=0; i<PI.pkthdr.caplen; i++)
                printf("%02x ",Packet[i]);
        }
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
    int i;

    while(TRUE)
    {
        if(pcap_next_ex(hpcap,&pkthdr,&recvBuff)>0)
        {
            /*
                        printf("数据包大小:%d\n\n ",pkthdr->caplen);
                        printf("接收方MAC:");
                        for(i=0; i<6; i++)
                            printf("%02x ",recvBuff[i]);
                        puts("");
                        printf("发送方MAC:");
                        for(; i<12; i++)
                            printf("%02x ",recvBuff[i]);
                        puts("");
                        printf("帧类型:0x%02x%02x\n",recvBuff[i],recvBuff[i+1]);
                        printf("\n----------------------------------------------\n");
            */
           // printf("抓取第%d个数据包。\n\n",++i);
            PI.Packet=recvBuff;
            PI.pkthdr=*pkthdr;
            CloseHandle((hFilterThread=CreateThread(NULL,0,filter,(PVOID)&PI,0,NULL)));
        }
    }

    return 0;
}

BOOL SendARPPacket(pcap_t *hpcap,char *devName,char *ARPPacket,int packetsize)
{
    int n=0;
    while(1)
        if(pcap_sendpacket(hpcap,ARPPacket,packetsize)==0)
        {
            printf("已发送%d个数据包。\r",++n);
            getch();
        }
        else
        {
            printf("数据包发送失败。\n");
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

int main()
{
    char devName[100];
    char myIPAddress[16];
    char ARPPacket[42];
    char srcMAC[7]= {0},desMAC[7]= {0};
    char query[20];
    pcap_t *hpcap=NULL;
    HANDLE hSnifferThread;
    PacketInfo PI;
    unsigned long temp;

    if(ChooseDev(devName,sizeof(devName),myIPAddress)!=TRUE)
    {
        printf("获取网卡失败。\n");
        getch();
        return -1;
    }
    if(GetAdapterMAC(myIPAddress,srcMAC)!=TRUE)
    {
        printf("获取网卡MAC失败。\n");
        getch();
        return -1;
    }
    if((hpcap=OpenAdapter(devName))==NULL)
    {
        printf("网卡打开出错。\n");
        getch();
        return -1;
    }

    memset(desMAC,0xff,6);      //广播
    memset(query,NULL,sizeof(query));
    printf("请输入要查询的IP:");
    fflush(stdin);
    gets(query);

    Fill_ARPPACKET(ARPPacket,sizeof(ARPPacket),desMAC,query,srcMAC,myIPAddress,2);
    int i;
    for(i=0; i<sizeof(ARPPacket); i++)
        printf("%02x ",ARPPacket[i]);
    puts("\n");
    //填充传递参数
    PI.hpcap=hpcap;
    memcpy(PI.srcMAC,srcMAC,6);
    temp=inet_addr(query);
    memcpy(PI.desIP,(char *)&temp,4);
    memcpy(PI.desMAC,desMAC,6);
    temp=inet_addr(myIPAddress);
    memcpy(PI.myIP,(char *)&temp,4);

    CloseHandle((hSnifferThread=CreateThread(NULL,0,Ether_Sniffer,(PVOID)&PI,0,NULL)));
    SendARPPacket(hpcap,devName,ARPPacket,sizeof(ARPPacket));
    while(1) Sleep(111);

    return 0;
}










