#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <pthread.h>

#include "async.h"
#include "mempool.h"

#define STACK_EMPTY -2

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
