#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <winsock.h>

#define MAX_SIZE 1024
#define SERVICEPORT 7777

#pragma comment(lib,"ws2_32.lib")

typedef struct
{
	struct sockaddr_in Address;
	SOCKET Socket;
}ClientSocket;

bool sendflag=false;
char SendBuff[MAX_SIZE];

DWORD WINAPI Input(PVOID Parameter)
{
	while(true)
	{
		fflush(stdin);
		gets(SendBuff);
		sendflag=true;
	}
	return 0;
}

DWORD WINAPI New_Client(PVOID Parameter)
{
	ClientSocket CS=*(ClientSocket *)Parameter;
	struct timeval timeout;
	fd_set readfd,writefd;
	char RecvBuff[MAX_SIZE];

    timeout.tv_sec=0;
	timeout.tv_usec=500;
	printf("客户端%s接入。\n",inet_ntoa(CS.Address.sin_addr));
	while (true)
	{
		FD_ZERO(&readfd);
		FD_ZERO(&writefd);
		FD_SET(CS.Socket,&readfd);
		FD_SET(CS.Socket,&writefd);

		if(select(-1,&readfd,&writefd,NULL,&timeout)>0)
		{
			if(FD_ISSET(CS.Socket,&readfd))
			{
				memset(RecvBuff,NULL,sizeof(RecvBuff));
				if(recv(CS.Socket,RecvBuff,sizeof(RecvBuff),0)<=0)
				{
					printf("客户端%s断开连接。\n",inet_ntoa(CS.Address.sin_addr));
					return -1;
				}
				printf("来自%s的消息:\n%s\n",inet_ntoa(CS.Address.sin_addr),RecvBuff);
			}
			if(FD_ISSET(CS.Socket,&writefd))
			{
				if(sendflag)
				{
					sendflag=false;
					if(send(CS.Socket,SendBuff,sizeof(SendBuff),0)<=0)
					{
						printf("客户端%s断开连接。\n",inet_ntoa(CS.Address.sin_addr));
				     	return -1;
					}
					memset(SendBuff,NULL,sizeof(SendBuff));
				}
			}
		}
	}

	return 0;
}

int main(int argc,char *argv[])
{
	ClientSocket CS;
	WSADATA wsa;
	struct sockaddr_in ServerAddress;
	SOCKET ServerSocket=INVALID_SOCKET;
	int AddressLength;
	HANDLE hThread;

	memset(&ServerAddress,NULL,sizeof(ServerAddress));
	ServerAddress.sin_family=AF_INET;
	ServerAddress.sin_addr.s_addr=INADDR_ANY;
	ServerAddress.sin_port=htons(SERVICEPORT);

	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
	{
		printf("Socket初始化失败。\n");
		getch();
		return -1;
	}
	if((ServerSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
	{
		printf("创建套接字失败。\n");
		getch();
		return -1;
	}
	if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))!=0)
	{
		printf("绑定本地地址失败。\n");
		getch();
		return -1;
	}
	if(listen(ServerSocket,SOMAXCONN)!=0)
	{
		printf("套接字转被动模式失败。\n");
		getch();
		return -1;
	}
	printf("The service started!\n");
	AddressLength=sizeof(CS.Address);
	CloseHandle((hThread=CreateThread(NULL,0,Input,NULL,0,NULL)));
	while(true)
	{
		memset(&CS,NULL,sizeof(ClientSocket));
		CS.Socket=accept(ServerSocket,(struct sockaddr *)&CS.Address,&AddressLength);
		CloseHandle((hThread=CreateThread(NULL,0,New_Client,(PVOID)&CS,0,NULL)));
		Sleep(1);
	}

	return 0;
}

/*
int PASCAL FAR select( int nfds, fd_set FAR* readfds,　fd_set FAR* writefds, fd_set FAR* exceptfds,　const struct timeval FAR* timeout);
nfds：是一个整数值，是指集合中所有文件描述符的范围，即所有文件描述符的最大值加1，不能错！在Windows中这个参数的值无所谓，可以设置不正确。
readfds：（可选）指针，指向一组等待可读性检查的套接口。
writefds：（可选）指针，指向一组等待可写性检查的套接口。
exceptfds：（可选）指针，指向一组等待错误检查的套接口。
timeout：select()最多等待时间，对阻塞操作则为NULL。
select()调用返回处于就绪状态并且已经包含在fd_set结构中的描述字总数；如果超时则返回0；否则的话，返回SOCKET_ERROR错误，应用程序可通过WSAGetLastError获取相应错误代码。
当返回为-1时，所有描述符集清0。
当返回为0时，超时不修改任何描述符集。
当返回为非0时，在3个描述符集里，依旧是1的位就是准备好的描述符。这也就是为什么，每次用select后都要用FD_ISSET的原因。
*/
