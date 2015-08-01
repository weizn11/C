#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "AVL.h"
#include "GUI.h"

int EnQueue(Queue *Q, QUEUE_DATATYPE elem)
{
	QNode *tmp = NULL;

	tmp = (QNode *)malloc(sizeof(QNode));
	if (tmp == NULL)
	{
		print("申请队列空间失败.", RED);
		return 0;
	}
	memset(tmp, NULL, sizeof(QNode));
	if (Q->start == NULL)
	{
		Q->start = Q->end = tmp;
	}
	else
	{
		Q->end->next = tmp;
		Q->end = tmp;
	}
	Q->end->data = elem;
	Q->queuesize++;

	return 1;
}

int DeQueue(Queue *Q, QUEUE_DATATYPE *elem)
{
	QNode *tmp = NULL;

	if (Q->start == NULL || Q->queuesize == 0) return 0;     //队列已空
	*elem = Q->start->data;
	Q->queuesize--;
	tmp = Q->start;
	Q->start = Q->start->next;
	free(tmp);

	return 1;
}


#endif       //_QUEUE_H_