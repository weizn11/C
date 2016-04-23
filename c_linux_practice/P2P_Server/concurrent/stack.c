#include <stdlib.h>
#include <string.h>

#include "stack.h"

int init_stack(STACK_INFO *StackInfo)
{
    pthread_mutex_init(&StackInfo->mutex,NULL);
    StackInfo->initFlag=1;

    return 0;
}

int push_stack(STACK_INFO *StackInfo,STACK_DATA_TYPE data)
{
    STACK_NODE *NewNode = NULL;

    if (StackInfo == NULL) return -1;
    if(StackInfo->initFlag==0)
        return -2;

    pthread_mutex_lock(&StackInfo->mutex);
    if(StackInfo->mempool==NULL)
    {
        StackInfo->mempool=create_mempool(sizeof(STACK_NODE),STACK_MEMPOOL_MAXIMUM_CELL);
        if(StackInfo->mempool==NULL)
        {
            pthread_mutex_unlock(&StackInfo->mutex);
            return -3;
        }
    }
    NewNode = (STACK_NODE *)mempool_alloc(StackInfo->mempool);
    if (NewNode == NULL)
    {
        pthread_mutex_unlock(&StackInfo->mutex);
        return -3;
    }
    memset(NewNode,NULL,sizeof(STACK_NODE));
    if (StackInfo->top == NULL)
        StackInfo->top = StackInfo->bottom = NewNode;
    else
    {
        NewNode->next = StackInfo->top;
        StackInfo->top = NewNode;
    }

    NewNode->data = data;
    StackInfo->count++;
    pthread_mutex_unlock(&StackInfo->mutex);

    return 0;
}

int pop_stack(STACK_INFO *StackInfo,STACK_DATA_TYPE *data)
{
    STACK_NODE *tmp = NULL;
    if (StackInfo == NULL) return -1;

    pthread_mutex_lock(&StackInfo->mutex);
    if (StackInfo->top == NULL || StackInfo->mempool==NULL)
    {
        pthread_mutex_unlock(&StackInfo->mutex);
        return STACK_EMPTY;
    }

    *data = StackInfo->top->data;
    tmp = StackInfo->top;
    StackInfo->top = StackInfo->top->next;
    mempool_free(StackInfo->mempool,tmp);
    StackInfo->count--;
    pthread_mutex_unlock(&StackInfo->mutex);

    return 0;
}
