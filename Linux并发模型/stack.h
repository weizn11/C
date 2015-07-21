#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <pthread.h>

#include "async.h"
#include "mempool.h"

#define STACK_EMPTY -2

/*一个栈元素的数据类型为IO_OPERATION_DATA_NODE链表节点指针和其中需要操作的客户端结构体索引地址
这个位置是空闲位置，可被新客户端占用
*/
typedef struct
{
    IO_OPERATION_DATA_NODE *pIONode;
    int index;
}STACK_DATA_TYPE;

typedef struct Stack_Node
{
	STACK_DATA_TYPE data;
	struct Stack_Node *next;
}STACK_NODE;

typedef struct
{
    int initFlag;
	unsigned int count;
	MEMPOOL_LIST *mempool;
	pthread_mutex_t mutex;
	STACK_NODE *top;
	STACK_NODE *bottom;
}STACK_INFO;

int init_stack(STACK_INFO *StackInfo);
int push_stack(STACK_INFO *StackInfo, STACK_DATA_TYPE data);
int pop_stack(STACK_INFO *StackInfo, STACK_DATA_TYPE *data);

#endif // STACK_H_INCLUDED
