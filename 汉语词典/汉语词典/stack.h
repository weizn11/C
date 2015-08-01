#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "GUI.h"

int InitStack(Stack *S)
{
	if (S->base != NULL) return 1;                                                  //栈结构已存在,不需要再创造
	S->base = S->top = (STACK_DATATYPE *)malloc(sizeof(STACK_DATATYPE)* 5);           //初始化容量为5的栈
	if (S->base == NULL)
	{
		print("malloc error.", RED);
		return 0;
	}
	S->stacksize = 5;

	return 1;
}

int Push(Stack *S, STACK_DATATYPE elem)
{
	if (S->top - S->base == S->stacksize - 1)
	{
		//需要为栈申请新的存储空间
		S->base = (STACK_DATATYPE *)realloc(S->base, sizeof(STACK_DATATYPE)*++S->stacksize);
		if (S->base == NULL)
		{
			print("申请新的栈空间失败.", RED);
			return 0;
		}
		S->top = S->base + (S->stacksize - 2);
	}
	*S->top = elem;           //数据入栈
	S->top++;               //栈顶指针向后移
	return 1;
}

int StackEmpty(Stack *S)
{
	return S->top - S->base == 0 ? 1 : 0;
}

int Pop(Stack *S, STACK_DATATYPE *elem)
{
	if (StackEmpty(S)) return 0;      //栈已空
	S->top--;                        //栈顶指针向前移动一单位
	*elem = *S->top;                   //栈顶元素出栈
	return 1;
}

#endif    //_STACK_H_

