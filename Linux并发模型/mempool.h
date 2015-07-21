#ifndef MEMPOOL_H_INCLUDED
#define MEMPOOL_H_INCLUDED

#include <pthread.h>

#include "async.h"

#define IODATA_MEMPOOL_MAXIMUM_CELL 2048     //IO_OPERATION_DATA结构体在内存池中需要申请的数量
#define STACK_MEMPOOL_MAXIMUM_CELL 2048      //栈元素在内存池中需要申请的数量

typedef IO_OPERATION_DATA MEMPOOL_DATA_TYPE;

typedef struct _MEMPOOL_LIST_
{
    void **pMempoolCell;        //指向内存地址的指针数组
    unsigned long int total;    //总共可用的内存数
    unsigned long int surplus;  //剩余可用的内存数
    unsigned long int cellSize; //每个内存单元的大小
    pthread_mutex_t mutex;
    struct _MEMPOOL_LIST_ *next;
}MEMPOOL_LIST;

MEMPOOL_LIST *create_mempool(unsigned long int dataSize,unsigned long int dataCount);
void *mempool_alloc(MEMPOOL_LIST *pMempool);
int mempool_free(MEMPOOL_LIST *pMempool,void *addr);

#endif // MEMPOOL_H_INCLUDED
