#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <windows.h>

//词条结构体
typedef struct
{
	char word[50];               //词条名
	char property[20];           //词性
	char pronunciation[50];      //读音
	char describe[100];          //释义
}WORD_INFO;
/*------------------------AVL----------------------------------------*/
//索引结构体
typedef struct
{
	int ASCII;                //单词ASCII之和
	LARGE_INTEGER Start;      //文件内的起始位置
	unsigned int Count;       //相同ASCII和的词条数目
}INDEXDATA;

//AVL树节点元素
typedef struct BiTree
{
	int balance;              //平衡因子
	INDEXDATA data;           //节点数据
	struct BiTree *left;
	struct BiTree *right;
	struct BiTree *father;    //指向双亲节点
} AVL;

AVL *AVL_ROOT = NULL;                  //AVL树根指针
/*------------------------Stack----------------------------------------*/
#define STACK_DATATYPE AVL*          //栈元素类型为AVL节点指针
typedef struct
{
	STACK_DATATYPE *top;             //栈顶指针
	STACK_DATATYPE *base;            //栈底指针
	int stacksize;             //栈容量
}Stack;

Stack S;                      //栈结构变量
/*------------------------Queue----------------------------------------*/
#define QUEUE_DATATYPE AVL*
typedef struct QueueNode
{
	QUEUE_DATATYPE data;
	struct QueueNode *next;
}QNode;

typedef struct
{
	QNode *start;               //队列起始地址
	QNode *end;                 //队列末尾地址
	int queuesize;              //队列节点个数
}Queue;

/*----------------------------------------------------------------------*/

HANDLE hDataFile;                       //数据库文件句柄
HANDLE hIndexFile;                      //索引文件句柄
LARGE_INTEGER DataFileSize;             //数据库文件大小

const char *IndexFilePath = "IndexFile";    //索引文件路径
const char *DataFilePath = "DataFile";      //数据库文件路径

#endif        //_GLOBAL_H_