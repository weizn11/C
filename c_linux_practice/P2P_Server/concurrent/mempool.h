#ifndef MEMPOOL_H_INCLUDED
#define MEMPOOL_H_INCLUDED

#include <pthread.h>

#include "async.h"

#define IODATA_MEMPOOL_MAXIMUM_CELL 2048
#define STACK_MEMPOOL_MAXIMUM_CELL 2048
#define RECV_MEMPOOL_MAXIMUM_CELL 2048

typedef IO_OPERATION_DATA MEMPOOL_DATA_TYPE;

typedef struct _MEMPOOL_LIST_
{
    void **pMempoolCell;
    unsigned long int total;
    unsigned long int surplus;
    unsigned long int cellSize;
    pthread_mutex_t mutex;
    struct _MEMPOOL_LIST_ *next;
}MEMPOOL_LIST;

MEMPOOL_LIST *create_mempool(unsigned long int dataSize,unsigned long int dataCount);
void *mempool_alloc(MEMPOOL_LIST *pMempool);
int mempool_free(MEMPOOL_LIST *pMempool,void *addr);

#endif // MEMPOOL_H_INCLUDED
