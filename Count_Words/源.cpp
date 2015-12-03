#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>

using namespace std;

#define BUF_SIZE 5000
#define TRIE_CHILD 10
#define WORD_SIZE 100
#define ALLOC_WORD_ARRAY_LEN 5000000
#define ALLOC_NODE_WORD_ARRAY_LEN 3


typedef struct _WORD_INFO_
{
	char word[WORD_SIZE];
	int count;
} WORD_INFO;

typedef struct _TRIE_TREE_NODE_
{
	WORD_INFO *pWordArray;
	int arrLen;
	int surArrayLen;
	struct _TRIE_TREE_NODE_ *child[TRIE_CHILD];
} TRIE_NODE;

char *pChineseArray = NULL;
int chineseArrayLen = 0;
TRIE_NODE *pRoot = NULL;
WORD_INFO *pWordArray = NULL;
int wordArrayLen = 0;
int surArrayLen = 0;

int startTime = 0;

int input_chinese()
{
	FILE *file = NULL;
	char inputBuffer[BUF_SIZE];
	char *pWord = NULL;
	int i;

	file = fopen("data_51_100.txt", "rb");
	if (file == NULL) exit(-1);

	pChineseArray = (char *)malloc(sizeof(char)* 100 * 1024 * 1024);
	if (pChineseArray == NULL) exit(-1);
	memset(pChineseArray, NULL, sizeof(char)* 100 * 1024 * 1024);

	while (!feof(file))
	{
		memset(inputBuffer, NULL, sizeof(inputBuffer));
		fgets(inputBuffer, sizeof(inputBuffer)-1, file);
		if (inputBuffer[strlen(inputBuffer) - 1] == '\n')
			inputBuffer[strlen(inputBuffer) - 1] = NULL;
		for (i = 0; inputBuffer[i] != NULL; i++)
		{
			if ((unsigned char)inputBuffer[i] <= 128)
				inputBuffer[i] = ' ';
		}

		pWord = strtok(inputBuffer, " ");
		while (pWord)
		{
			if (strlen(pWord) == 2)
			{
				if ((unsigned char)pWord[0]>128 && (unsigned char)pWord[1]>128)
				{
					//strcat(pChineseArray,pWord); //汉字
					pChineseArray[chineseArrayLen] = pWord[0];
					pChineseArray[chineseArrayLen + 1] = pWord[1];
					chineseArrayLen += 2;
				}
			}
			pWord = strtok(NULL, " ");
		}
		pChineseArray[chineseArrayLen] = ' ';
		chineseArrayLen += 1;
	}
	fclose(file);

	return 0;
}


WORD_INFO *search_word_from_array(WORD_INFO *pHead, int n, char *word, int len)
{
	int i;
	WORD_INFO *pNode = NULL;

	for (i = 0; i<n; i++)
	{
		if (!strncmp(pHead[i].word, word, len))
			return &pHead[i];
	}

	return NULL;
}

int create_trie_tree(int n)
{
	int i, nodeCount = 0, index;
	char *pWord = NULL;
	TRIE_NODE *pNode = NULL;
	WORD_INFO *pWordInfo = NULL;

	pRoot = (TRIE_NODE *)malloc(sizeof(TRIE_NODE));
	if (pRoot == NULL) return -1;
	memset(pRoot, NULL, sizeof(TRIE_NODE));

	pWord = pChineseArray;
	while (*pWord != NULL)
	{
		for (pNode = pRoot, i = 0; i<n * 2 && *pWord != ' ' && *pWord!=NULL; i++, pWord++)
		{
			index = (unsigned char)*pWord%TRIE_CHILD;
			if (pNode->child[index] == NULL)
			{
				nodeCount++;
				pNode->child[index] = (TRIE_NODE *)malloc(sizeof(TRIE_NODE));
				if (pNode->child[index] == NULL)
				{
					puts("heap overflow");
					return -1;
				}
				memset(pNode->child[index], NULL, sizeof(TRIE_NODE));
			}
			pNode = pNode->child[index];
			if (i == n * 2 - 1)
			{
				pWordInfo = NULL;
				if (pNode->pWordArray == NULL || !(pWordInfo = search_word_from_array(pNode->pWordArray, pNode->arrLen, pWord - i, n * 2)))
				{
					if (pNode->surArrayLen == 0)
					{
						pNode->pWordArray = (WORD_INFO *)realloc(pNode->pWordArray, sizeof(WORD_INFO)*\
							(pNode->arrLen + ALLOC_NODE_WORD_ARRAY_LEN));
						if (pNode->pWordArray == NULL)
						{
							puts("heap overflow");
							return -1;
						}
						memset(&pNode->pWordArray[pNode->arrLen], NULL, sizeof(WORD_INFO)*ALLOC_NODE_WORD_ARRAY_LEN);
						pNode->surArrayLen = ALLOC_NODE_WORD_ARRAY_LEN;
					}

					strncat(pNode->pWordArray[pNode->arrLen].word, pWord - i, n * 2);
					pWordInfo = &pNode->pWordArray[pNode->arrLen];
					pNode->arrLen++;
					pNode->surArrayLen--;
				}
				pWordInfo->count++;
				//printf("%s:%d\n",pWordInfo->word,pWordInfo->count);
			}

		}
		if (*pWord == ' ')
		{
			pWord++;
			continue;
		}
		if (*pWord != NULL)
			pWord -= (n * 2 - 2);
	}

	printf("共创建%d个节点\n", nodeCount);
	return 0;
}

int trie_child_empty(TRIE_NODE *pCurrRoot)
{
	int i;

	for (i = 0; i<TRIE_CHILD; i++)
	if (pCurrRoot->child[i] != NULL)
		return 0;

	return 1;
}

int pre_order_traverse(TRIE_NODE *pCurrRoot, int &depth, int n)
{
	int i;
	char word[WORD_SIZE];

	if (pCurrRoot == NULL) return -1;

	if (pCurrRoot != pRoot)
	{
		depth++;
		if (depth == n * 2)
		{
			//遍历到叶子节点
			//输出大于1的词组
			//printf("%s\t%d\n",word,pCurrRoot->count);
			for (i = 0; i<pCurrRoot->arrLen; i++)
			{
				if (pCurrRoot->pWordArray[i].count<2)continue;

				if (surArrayLen == 0)
				{
					pWordArray = (WORD_INFO *)realloc(pWordArray, sizeof(WORD_INFO)*(wordArrayLen + ALLOC_WORD_ARRAY_LEN));
					if (pWordArray == NULL) return -1;
					memset(pWordArray + wordArrayLen, NULL, sizeof(WORD_INFO)*ALLOC_WORD_ARRAY_LEN);

					surArrayLen = ALLOC_WORD_ARRAY_LEN;
				}
				pWordArray[wordArrayLen] = pCurrRoot->pWordArray[i];
				wordArrayLen++;
				surArrayLen--;
			}
		}
	}
	else
	{
		depth = 0;
	}
	for (i = 0; i<TRIE_CHILD; i++)
	{
		pre_order_traverse(pCurrRoot->child[i], depth, n);
	}
	depth--;

	return 0;
}

int heap_adjust(WORD_INFO *arr, int rootIndex, int endIndex)
{
	//建立大根堆
	int childIndex;
	WORD_INFO temp;
	for (childIndex = rootIndex * 2 + 1; childIndex <= endIndex; rootIndex = childIndex, childIndex = rootIndex * 2 + 1)
	{
		if (childIndex<endIndex && arr[childIndex].count>arr[childIndex + 1].count)
			childIndex++;
		if (arr[rootIndex].count>arr[childIndex].count)
		{
			temp = arr[rootIndex];
			arr[rootIndex] = arr[childIndex];
			arr[childIndex] = temp;
		}
		else
			break;
	}
	return 0;
}

int heap_sort(WORD_INFO *arr, int n)
{
	int i;
	WORD_INFO temp;
	for (i = n / 2; i >= 0; i--)
	{
		heap_adjust(arr, i, n - 1);
	}

	for (i = n - 1; i >= 0; i--)
	{
		temp = arr[0];
		arr[0] = arr[i];
		arr[i] = temp;
		heap_adjust(arr, 0, i - 1);
	}
	return 0;
}

int output_file(char *fileName)
{
	int i;
	FILE *file = NULL;

	file = fopen(fileName, "wt");
	if (file == NULL)
	{
		puts("create file failed.");
		return -1;
	}

	for (i = 0; i<wordArrayLen; i++)
	{
		fprintf(file, "%s\t%d\n", pWordArray[i].word, pWordArray[i].count);
	}
	fclose(file);

	return 0;
}

int count(int n)
{
	int depth;
	int time1;
	time1 = time(NULL);
	create_trie_tree(n);
	printf("字典树建立完毕:%d s\n", time(NULL) - time1);
	time1 = time(NULL);
	pre_order_traverse(pRoot, depth, n);
	printf("遍历树完毕:%d s\n", time(NULL) - time1);
	if (pWordArray == NULL)
	{
		puts("arr empty");
		return -1;
	}
	time1 = time(NULL);
	heap_sort(pWordArray, wordArrayLen);
	printf("排序完毕:%d s\n", time(NULL) - time1);
	output_file("word.txt");
}

int test()
{
	int i;
	int n = 0;

	for (i = 0; i<chineseArrayLen; i += 2)
	if (!strncmp(&pChineseArray[i], "的", 2))
		n++;
	printf("“的”：%d\n", n);
	//getch();
	return 0;
}

int main()
{
	int n;

	printf("统计单词数:");
	scanf("%d", &n);
	startTime = time(NULL);
	input_chinese();
	printf("读取文件花费:%d秒\n", (time(NULL) - startTime));
	//test();
	count(n);
	printf("\n共花费:%d秒\n", (time(NULL) - startTime));

	_getch();
	//puts(pChineseArray);
	return 0;
}
