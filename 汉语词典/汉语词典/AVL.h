#ifndef _AVL_H_
#define _AVL_H_

#include <windows.h>
#include <string.h>

#include "global.h"
#include "FileOper.h"
#include "stack.h"
#include "GUI.h"

int Depth(AVL *root)
{
	//计算树的深度。
	if (root == NULL)
		return 0;
	int i, j;

	if (root->left)
		i = Depth(root->left);
	else
		i = 0;

	if (root->right)
		j = Depth(root->right);
	else
		j = 0;

	return i>j ? i + 1 : j + 1;
}

void updateBF(AVL *NewNode)
{
	//更新平衡因子
	if (!NewNode || !NewNode->father) return;
	AVL *CurrNode = NewNode;
	AVL *PreNode = CurrNode->father;

	if (CurrNode == PreNode->left)
	{
		//指针从左路回溯
		PreNode->balance++;
		if (PreNode->balance == 2) return;
		if (PreNode->left && PreNode->right)   //遇到有两个孩子的节点
		if (PreNode->balance <= 0)
			return;
	}
	else
	{
		//指针从右路回溯
		PreNode->balance--;
		if (PreNode->balance == -2) return;
		if (PreNode->left && PreNode->right)
		if (PreNode->balance >= 0)
			return;
	}
	updateBF(PreNode);

	return;
}

void RR_Rotate(AVL **root, AVL *centreNode)
{
	//向右旋转,RR型。
	AVL *temp = NULL;

	if (centreNode->right)
	{
		//中心节点的右节点不为空
		centreNode->father->left = centreNode->right;     //将中心节点的右子树连接到双亲节点的左子树上。
		centreNode->right->father = centreNode->father;
	}
	else
		//中心节点的右节点为空
		centreNode->father->left = NULL;

	temp = centreNode->father->father;
	if (centreNode->father->father != NULL)
	if (centreNode->father->father->left == centreNode->father)
		centreNode->father->father->left = centreNode;
	else
		centreNode->father->father->right = centreNode;
	else
		*root = centreNode;

	centreNode->right = centreNode->father;         //当前节点的双亲成为其右子树。
	centreNode->father->father = centreNode;
	centreNode->father->balance -= 2;
	centreNode->father = temp;
	centreNode->balance--;

	return;
}

void LL_Rotate(AVL **root, AVL *centreNode)
{
	//向左旋转,LL型
	AVL *temp = NULL;

	if (centreNode->left)
	{
		//中心节点的左节点不为空
		centreNode->father->right = centreNode->left;    //将中心节点的左子树连接到双亲的右子树上。
		centreNode->left->father = centreNode->father;   //中心节点的左子树的双亲改为中心节点的双亲。
	}
	else
		centreNode->father->right = NULL;

	temp = centreNode->father->father;
	if (centreNode->father->father != NULL)
	if (centreNode->father->father->left == centreNode->father)
		centreNode->father->father->left = centreNode;
	else
		centreNode->father->father->right = centreNode;
	else
		*root = centreNode;

	centreNode->left = centreNode->father;         //将中心节点的左孩子改为中心节点的双亲
	centreNode->left->father = centreNode;         //将中心节点左孩子的双亲更改为中心节点。
	centreNode->balance++;
	centreNode->father->balance += 2;
	centreNode->father = temp;

	return;
}

void RL_Rotate(AVL **root, AVL *centreNode)
{
	//先右后左旋转
	AVL *temp = NULL;

	temp = centreNode->father;
	centreNode->father->right = centreNode->left;
	centreNode->left->father = centreNode->father;
	centreNode->left->balance--;
	centreNode->balance--;

	if (centreNode->left->right)
	{
		//中心节点的左节点的右孩子不为空。
		temp->right->right->father = centreNode;
	}
	centreNode->left = temp->right->right;
	temp->right->right = centreNode;
	centreNode->father = temp->right;
	if (temp->right->balance == -2)
		temp->right->balance++;
	LL_Rotate(root, temp->right);

	return;
}

void LR_Rotate(AVL **root, AVL *centreNode)
{
	//先左后右旋转，LR型。
	AVL *temp = NULL;

	temp = centreNode->father;
	centreNode->father->left = centreNode->right;
	centreNode->right->father = centreNode->father;
	centreNode->right->balance++;
	centreNode->balance++;

	if (centreNode->right->left)
	{
		//中心节点的左节点的右孩子不为空。
		temp->left->left->father = centreNode;
	}
	centreNode->right = temp->left->left;
	temp->left->left = centreNode;
	centreNode->father = temp->left;
	if (temp->left->balance == 2)
		temp->left->balance--;
	RR_Rotate(root, temp->left);

	return;
}

void adjustAVL(AVL **root, AVL *NewNode)
{
	if (!NewNode) return;
	AVL *CurrNode = NewNode;
	AVL *PreNode = NULL;

	while (CurrNode->father != NULL && CurrNode->father->balance != 2 && CurrNode->father->balance != -2)
	{
		PreNode = CurrNode;
		CurrNode = CurrNode->father;
	}
	if (!CurrNode->father) return;

	if (CurrNode->father->left == CurrNode)
	{
		//自己是双亲的左节点
		if (CurrNode->left == PreNode)
			//指针从左路回溯。
			RR_Rotate(root, CurrNode);
		else
			//指针从右路回溯
			LR_Rotate(root, CurrNode);
	}
	else
	{
		//自己是双亲的右节点
		if (CurrNode->right == PreNode)
			//指针从右路回溯
			LL_Rotate(root, CurrNode);
		else
			//指针从左路回溯
			RL_Rotate(root, CurrNode);
	}

	return;
}

int InsertIndex(AVL **root, INDEXDATA *elem)
{
	AVL *CurrNode = NULL;
	AVL *PreNode = NULL;
	AVL *NewNode = NULL;

	if (*root == NULL)                  //如果根节点不存在则创建
	{
		if ((*root = (AVL *)malloc(sizeof(AVL))) == NULL)
			return 0;
		memset(*root, NULL, sizeof(AVL));
		elem->Count = 1;               //此ASCII和的词条个数
		(*root)->data = *elem;
		return 1;
	}

	CurrNode = *root;
	while (CurrNode)
	{
		PreNode = CurrNode;
		if (elem->ASCII<CurrNode->data.ASCII)
			CurrNode = CurrNode->left;
		else if (elem->ASCII>CurrNode->data.ASCII)
			CurrNode = CurrNode->right;
		else
		{
			//已存在该索引
			CurrNode->data.Count++;                  //此类型的词条个数加一
			elem->Start = CurrNode->data.Start;
			elem->Count = CurrNode->data.Count;
			return 2;    //表示已存在该元素
		}
	}
	//索引中无此节点，添加新的节点。
	if ((NewNode = (AVL *)malloc(sizeof(AVL))) == NULL)        //为新节点开辟空间
		return 0;
	memset(NewNode, NULL, sizeof(AVL));                     //初始化新节点中的数据
	if (GetFileSizeEx(hDataFile, &DataFileSize) != TRUE)      //获取数据库文件总大小
		return 0;
	elem->Start = DataFileSize;                             //将新添加的词条追加到数据库文件尾部
	elem->Count = 1;                                        //此类型的词条数目为一
	NewNode->data = *elem;                                  //为新节点赋值
	NewNode->father = PreNode;
	//插入新节点
	if (elem->ASCII<PreNode->data.ASCII)
		PreNode->left = NewNode;
	else
		PreNode->right = NewNode;
	updateBF(NewNode);          //更新AVL的平衡因子
	adjustAVL(root, NewNode);    //AVL自调节

	return 1;
}

int InsertIndexFromFile(AVL **root, INDEXDATA *elem)
{
	AVL *CurrNode = NULL;
	AVL *PreNode = NULL;
	AVL *NewNode = NULL;

	if (*root == NULL)  //如果根节点不存在则创建
	{
		if ((*root = (AVL *)malloc(sizeof(AVL))) == NULL)
			return 0;
		memset(*root, NULL, sizeof(AVL));
		(*root)->data = *elem;
		return 1;
	}

	CurrNode = *root;
	while (CurrNode)
	{
		PreNode = CurrNode;
		if (elem->ASCII<CurrNode->data.ASCII)
			CurrNode = CurrNode->left;
		else if (elem->ASCII>CurrNode->data.ASCII)
			CurrNode = CurrNode->right;
		else
		{
			//已存在该索引
			return 2;    //表示已存在该元素
		}
	}
	//索引中无此节点，添加新的节点。
	if ((NewNode = (AVL *)malloc(sizeof(AVL))) == NULL)       //为新节点开辟空间
		return 0;
	memset(NewNode, NULL, sizeof(AVL));                    //初始化新节点中的数据
	NewNode->data = *elem;                                 //为新节点赋值
	NewNode->father = PreNode;
	//插入新节点
	if (elem->ASCII<PreNode->data.ASCII)
		PreNode->left = NewNode;
	else
		PreNode->right = NewNode;
	updateBF(NewNode);          //更新AVL的平衡因子
	adjustAVL(root, NewNode);    //AVL自调节

	return 1;
}

int AlterIndexStartAddr(AVL *root, const LARGE_INTEGER *refer, int pattern)
{
	//中序遍历,第三个参数:0.有词条被添加.1.有词条被删除.
	if (root == NULL)
		return 0;
	AlterIndexStartAddr(root->left, refer, pattern);

	if (root->data.Start.QuadPart>refer->QuadPart)
	{
		if (pattern == 0)
		{
			//有词条被添加,部分索引起始地址后移.
			root->data.Start.QuadPart += sizeof(WORD_INFO);    //修改其他词条索引的起始地址
		}
		else
		{
			//有词条被删除,部分索引起始地址前移
			root->data.Start.QuadPart -= sizeof(WORD_INFO);
		}
		InsertIndexToFile(root->data, 0);                 //将修改结果存入文件中
	}

	AlterIndexStartAddr(root->right, refer, pattern);
	return 1;
}

INDEXDATA *SearchAVL(AVL *root, int ASCII)
{
	INDEXDATA *result = NULL;

	if (!root) return NULL;
	if (ASCII == root->data.ASCII) return &root->data;
	if (ASCII<root->data.ASCII)
	if ((result = SearchAVL(root->left, ASCII))) return result;
	if (ASCII>root->data.ASCII)
	if ((result = SearchAVL(root->right, ASCII))) return result;

	return NULL;
}

int PushAVLToStack(Stack *S, AVL *root)
{
	//先序遍历入栈
	if (!root) return 0;

	Push(S, root);
	if (root->left)
		PushAVLToStack(S, root->left);
	if (root->right)
		PushAVLToStack(S, root->right);

	return 1;
}

int DestroyAVL(AVL *root)
{
	AVL *tmp = NULL;

	if (!InitStack(&S))             //初始化栈
	{
		print("初始化栈失败.", RED);
		return 0;
	}
	PushAVLToStack(&S, root);      //AVL所有节点指针入栈
	AVL_ROOT = NULL;                //AVL根指针置空
	while (!StackEmpty(&S))
	{
		Pop(&S, &tmp);             //指针变量出栈
		free(tmp);                //释放内存
	}
	return 1;
}

int NodeCount(AVL *root)
{
	//统计树中有多少个节点
	int n = 0;
	if (root == NULL)
		return 0;
	n = NodeCount(root->left);
	n++;
	n += NodeCount(root->right);
	return n;
}

int WordCount(AVL *root)
{
	//统计索引包含了多少个词条
	int n = 0;
	if (root == NULL)
		return 0;
	n = WordCount(root->left);
	n += root->data.Count;
	n += WordCount(root->right);
	return n;
}

#endif          //_AVL_H_




