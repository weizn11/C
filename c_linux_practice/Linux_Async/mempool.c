#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mempool.h"

MEMPOOL_LIST *create_mempool(unsigned long int dataSize,unsigned long int dataCount)
{
    int i;
    void *pMempool=NULL;
    MEMPOOL_LIST *newNode=NULL;

    newNode=(MEMPOOL_LIST *)malloc(sizeof(MEMPOOL_LIST));
    if(newNode==NULL) return NULL;
    memset(newNode,NULL,sizeof(MEMPOOL_LIST));
    newNode->surplus=newNode->total=dataCount;
    newNode->cellSize=dataSize;
    pthread_mutex_init(&newNode->mutex,NULL);

    newNode->pMempoolCell=(void **)malloc(sizeof(void *)*dataCount);
    if(newNode->pMempoolCell==NULL)
    {
        free(newNode);
        return NULL;
    }
    memset(newNode->pMempoolCell,NULL,sizeof(void *)*dataCount);

    pMempool=malloc(dataSize*dataCount);
    if(pMempool==NULL)
    {
        free(newNode->pMempoolCell);
        free(newNode);
        return NULL;
    }
    memset(pMempool,NULL,dataSize*dataCount);

    for(i=0; i<dataCount; i++)
        newNode->pMempoolCell[i]=pMempool+i*dataSize;

    return newNode;
}

void *mempool_alloc(MEMPOOL_LIST *pMempool)
{
    void *addr=NULL;
    MEMPOOL_LIST *pMempoolListNode=NULL,*newNode=NULL;

    pthread_mutex_lock(&pMempool->mutex);
    for(pMempoolListNode=pMempool; pMempoolListNode->surplus==0 && pMempoolListNode->next!=NULL; \
            pMempoolListNode=pMempoolListNode->next);

    if(pMempoolListNode->surplus==0)
    {
        newNode=create_mempool(pMempool->cellSize,pMempool->total);
        if(newNode==NULL)
        {
            pthread_mutex_unlock(&pMempool->mutex);
            return NULL;
        }
        pMempoolListNode->next=newNode;
        pMempoolListNode=newNode;
    }

    addr=pMempoolListNode->pMempoolCell[pMempoolListNode->total-pMempoolListNode->surplus];
    pMempoolListNode->pMempoolCell[pMempoolListNode->total-pMempoolListNode->surplus]=NULL;
    pMempoolListNode->surplus--;
    pthread_mutex_unlock(&pMempool->mutex);

    return addr;
}

int mempool_free(MEMPOOL_LIST *pMempool,void *addr)
{
    MEMPOOL_LIST *pMempoolListNode=NULL;

    pthread_mutex_lock(&pMempool->mutex);
    for(pMempoolListNode=pMempool; pMempoolListNode->surplus==pMempoolListNode->total && \
            pMempoolListNode->next!=NULL; pMempoolListNode=pMempoolListNode->next);
    if(pMempoolListNode->surplus==pMempoolListNode->total)
    {
        pthread_mutex_unlock(&pMempool->mutex);
        return -1;
    }

    pMempoolListNode->pMempoolCell[pMempoolListNode->total-pMempoolListNode->surplus-1]=addr;
    pMempoolListNode->surplus++;
    pthread_mutex_unlock(&pMempool->mutex);

    return 0;
}










