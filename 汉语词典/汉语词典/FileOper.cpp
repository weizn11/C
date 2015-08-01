#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "global.h"
#include "FileOper.h"
#include "AVL.h"
#include "queue.h"
#include "GUI.h"

int ASCII_SUM(char *str);
int AdjustIndex(INDEXDATA _index);
int Query_Word(char *word, WORD_INFO *WI, LARGE_INTEGER *PCurrSize);
int Query();
int Add_Word();
int Alter_Word(LARGE_INTEGER CurrSize, WORD_INFO *WI);
int Delete_Word(LARGE_INTEGER CurrSize);
int Delete_Index(int ascii);
int OptimizeIndexFile();
void ShowFile();

int ASCII_SUM(char *str)
{
	int sum = 0;
	char *p = str;

	for (sum = 0; *p != NULL; p++)
		sum += (int)*p;           //计算单词的ASCII和
	return sum;
}

int AdjustIndex(INDEXDATA _index)
{
	DWORD tmp_dw;
	LARGE_INTEGER ReferSize;

	memset(&ReferSize, NULL, sizeof(LARGE_INTEGER));

	InsertIndexToFile(_index, 0);                 //先修改新词条的索引
	ReferSize = _index.Start;
	AlterIndexStartAddr(AVL_ROOT, &ReferSize, 0);

	return 1;
}

int Query_Word(char *word, WORD_INFO *WI, LARGE_INTEGER *PCurrSize)
{
	//返回值:1.查询成功。0.此词条未被收录。-1.查询出错。
	int word_ascii, n;
	DWORD tmp_dw;
	INDEXDATA *pIndex = NULL;
	WORD_INFO ReadBuff;
	LARGE_INTEGER MovSize, CurrSize;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));

	word_ascii = ASCII_SUM(word);                //先计算出要查找词语的ASCII码之和
	if ((pIndex = SearchAVL(AVL_ROOT, word_ascii)) == NULL)
		return 0;                              //此词语未被收录
	MovSize = pIndex->Start;
	MovSize.LowPart = SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);    //文件内部指针移至此类词条开始的位置
	for (n = pIndex->Count; n != 0; n--)
	{
		memset(&ReadBuff, NULL, sizeof(WORD_INFO));
		if (!ReadFile(hDataFile, &ReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return -1;
		if (!strcmp(ReadBuff.word, word))
		{
			//查询到此单词
			if (WI != NULL)
				memcpy(WI, &ReadBuff, sizeof(WORD_INFO));
			if (PCurrSize != NULL)
			{
				//获取当前文件内部指针位置
				CurrSize.LowPart = SetFilePointer(hDataFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_CURRENT);
				CurrSize.QuadPart -= sizeof(WORD_INFO);
				memcpy(PCurrSize, &CurrSize, sizeof(LARGE_INTEGER));
			}
			return 1;
		}
	}

	return 0;
}

int Query()
{
	LARGE_INTEGER CurrSize;
	WORD_INFO WI;
	int choose;
	char ch_temp[50], word[50], result[100];
	TCHAR TC_temp[50];

	memset(&WI, NULL, sizeof(WORD_INFO));
	memset(word, NULL, sizeof(word));
	memset(result, NULL, sizeof(result));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("请输入词名"), _T("查找词条"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memcpy(word, ch_temp, 50);

	switch (Query_Word(word, &WI, &CurrSize))
	{
	case 1:
		sprintf(result, "词名:%s", WI.word);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "读音:%s", WI.pronunciation);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "词性:%s", WI.property);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "释义:%s", WI.describe);
		print(result, BLACK);

		printf("1.修改\t2.删除\t0.返回\n");

		cover(40, 250, 150, 50);
		cover(40, 370, 150, 50);
		cover(30, 500, 190, 50);

		settextcolor(GREEN);
		setbkmode(TRANSPARENT);
		settextstyle(15, 15, NULL);
		outtextxy(80, 270, _T("修改"));
		outtextxy(80, 395, _T("删除"));
		outtextxy(80, 525, _T("返回"));
		while (!(choose = GetMouse()));
		if (choose == 1)
		{
			if (!Alter_Word(CurrSize, &WI))
			{
				print("词条修改失败.", RED);
			}
			text2_y = 222;                  //控制台消息输出坐标初始化
			cover(251, 222, 473, 378);
			sprintf(result, "词名:%s", WI.word);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "读音:%s", WI.pronunciation);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "词性:%s", WI.property);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "释义:%s", WI.describe);
			print(result, BLACK);
			print("词条修改成功.", GREEN);
		}
		else if (choose == 2)
		{
			text2_y = 222;                  //控制台消息输出坐标初始化
			cover(251, 222, 473, 378);
			if (Delete_Word(CurrSize) != 1)
			{
				print("删除词条失败.", RED);
			}
			else
			{
				memset(result, NULL, sizeof(result));
				sprintf(result, "已成功删除词条:%s", WI.word);
				print(result, GREEN);
			}
		}
		break;
	case 0:
		print("不存在此单词.", YELLOW);
		return 0;
	default:
		print("查询出错.", RED);
		return -1;
	}

	return 1;
}

int Add_Word()
{
	int InsertIndexResult = 0;
	INDEXDATA IndexData;            //AVL索引节点数据内容
	WORD_INFO word;                 //包含每个单词信息
	TCHAR TC_temp[100];
	char ch_temp[100];

	memset(&IndexData, NULL, sizeof(INDEXDATA));
	memset(&word, NULL, sizeof(WORD_INFO));
	memset(TC_temp, NULL, sizeof(TC_temp));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("请输入词名"), _T("添加词条"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memcpy(word.word, ch_temp, 50);

	IndexData.ASCII = ASCII_SUM(word.word);                    //计算单词ASCII和
	//printf("此单词的ASCII之和为:%d\n",IndexData.ASCII);
	if (Query_Word(word.word, NULL, NULL) == 1)
	{
		print("已收录此词条,请勿重复添加.", RED);
		return 2;
	}

	do
	{
		InputBox(TC_temp, 50, _T("请输入拼音"), _T("添加词条"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memcpy(word.pronunciation, ch_temp, 50);

	do
	{
		InputBox(TC_temp, 20, _T("请输入词性"), _T("添加词条"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memcpy(word.property, ch_temp, 20);

	do
	{
		InputBox(TC_temp, 100, _T("请输入释义"), _T("添加词条"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memcpy(word.describe, ch_temp, 100);

	if (!(InsertIndexResult = InsertIndex(&AVL_ROOT, &IndexData)))    //插入或修改索引节点
	{
		//第二个参数返回了新词条对应的索引数据
		printf("添加索引失败。", RED);
		return 0;
	}
	if (!InsertDataToFile(word, IndexData))
	{
		//将新的词条添加至数据库文件中,第二个参数是新词条所对应的索引.
		printf("写文件失败。", RED);
		return 0;
	}
	//当InsertIndexResult返回值为1时表明新添加的词条追加在数据库文件末尾，不需要对全部索引进行更新，否则需要。
	if (InsertIndexResult == 1)
	{
		if (!InsertIndexToFile(IndexData, 1))         //第二个参数1表示新添加的索引节点只需要追加在索引文件的末尾
		{
			printf("写入索引文件失败。", RED);
			return 0;
		}
	}
	else
	{
		//在索引中只是修改了节点数据,没有添加新索引.
		switch (AdjustIndex(IndexData))
		{
		case 0:
		{
				  printf("调整索引失败。", RED);
				  return 0;
		}
		case -1:
		{
				   printf("文件异常,请重新建立!", RED);
				   return 0;
		}
		default:
			break;
		}
	}
	return 1;
}

int Alter_Word(LARGE_INTEGER CurrSize, WORD_INFO *WI)
{
	DWORD tmp_dw;
	TCHAR TC_temp[100];
	char ch_temp[100];

	memset(TC_temp, NULL, sizeof(TC_temp));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("修改读音"), _T("修改词条"), CharToTCHAR(WI->pronunciation), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memset(WI->pronunciation, NULL, 50);
	memcpy(WI->pronunciation, ch_temp, 50);

	do
	{
		InputBox(TC_temp, 20, _T("修改词性"), _T("修改词条"), CharToTCHAR(WI->property), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memset(WI->property, NULL, 20);
	memcpy(WI->property, ch_temp, 20);

	do
	{
		InputBox(TC_temp, 100, _T("修改释义"), _T("修改词条"), CharToTCHAR(WI->describe), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //将TCHAR类型字符集转换为char类型
	} while (strlen(ch_temp)<1);
	memset(WI->describe, NULL, 100);
	memcpy(WI->describe, ch_temp, 100);

	SetFilePointer(hDataFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_BEGIN);
	if (!WriteFile(hDataFile, WI, sizeof(WORD_INFO), &tmp_dw, NULL))
		return 0;
	return 1;
}

int Delete_Word(const LARGE_INTEGER CurrSize)
{
	//返回值:1.删除成功.0.删除失败.-1.程序异常
	int word_ascii;
	DWORD tmp_dw;
	WORD_INFO WordReadBuff;
	INDEXDATA IndexReadBuff;
	INDEXDATA *PIndex = NULL;
	LARGE_INTEGER refer, WriteAddr, ReadAddr;
	LARGE_INTEGER MovSize = CurrSize;

	memset(&WordReadBuff, NULL, sizeof(WORD_INFO));
	memset(&IndexReadBuff, NULL, sizeof(INDEXDATA));
	memset(&refer, NULL, sizeof(LARGE_INTEGER));
	memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&ReadAddr, NULL, sizeof(LARGE_INTEGER));

	SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);
	if (!ReadFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
	{
		print("读取数据库文件出错.", RED);
		return 0;
	}
	word_ascii = ASCII_SUM(WordReadBuff.word);
	PIndex = SearchAVL(AVL_ROOT, word_ascii);
	if (PIndex == NULL)
	{
		print("未找到此词条索引.", RED);
		return -1;
	}
	refer = PIndex->Start;                 //保存当前词条起始地址
	if (PIndex->Count>1)
	{
		PIndex->Count--;                 //此词条类型的索引还存在,只是词条数目减一
		InsertIndexToFile(*PIndex, 0);    //修改文件内储存的相应索引
	}
	else
	{
		//此类型的词条被删完,索引随之要被删除.
		Delete_Index(word_ascii);
		//删除文件内相应索引后要销毁现有索引,初始化新索引.
		DestroyAVL(AVL_ROOT);
		InitIndex();
	}
	if (!AlterIndexStartAddr(AVL_ROOT, &refer, 1) && GetLastError() != 0)
	{
		print("更新索引失败.", RED);
		return 0;
	}

	//删除数据库文件中的词条信息
	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));
	GetFileSizeEx(hDataFile, &DataFileSize);                  //获取数据库文件大小
	WriteAddr = CurrSize;
	ReadAddr.QuadPart = WriteAddr.QuadPart + sizeof(WORD_INFO);
	if (ReadAddr.QuadPart == DataFileSize.QuadPart)
	{
		SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
		SetEndOfFile(hDataFile);
		return 1;
	}
	while (1)
	{
		SetFilePointer(hDataFile, ReadAddr.LowPart, &ReadAddr.HighPart, FILE_BEGIN);
		if (!ReadFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return 0;
		if (tmp_dw<1) break;
		SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
		if (!WriteFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return 0;
		WriteAddr.QuadPart += sizeof(WORD_INFO);
		ReadAddr.QuadPart += sizeof(WORD_INFO);
		if (ReadAddr.QuadPart == DataFileSize.QuadPart)
		{
			SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
			SetEndOfFile(hDataFile);
			return 1;
		}
	}

	return -1;
}

int Delete_Index(int ascii)
{
	DWORD tmp_dw;
	INDEXDATA ReadBuff;
	LARGE_INTEGER ReadAddr, CurrSize, WriteAddr;

	memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	memset(&ReadAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));
	memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));

	GetFileSizeEx(hIndexFile, &DataFileSize);                                      //获取索引文件大小
	SetFilePointer(hIndexFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_BEGIN);    //文件内部指针至于起始位置
	while (1)
	{
		if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
		{
			print("读取索引文件出错.", RED);
			return 0;
		}
		if (tmp_dw<1) break;
		if (ReadBuff.ASCII == ascii)
		{
			memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));
			CurrSize.LowPart = SetFilePointer(hIndexFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_CURRENT);   //获取当前文件指针
			ReadAddr = CurrSize;
			memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
			WriteAddr.QuadPart = ReadAddr.QuadPart - sizeof(INDEXDATA);            //向后退一个索引的长度
			if (ReadAddr.QuadPart == DataFileSize.QuadPart)
			{
				SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
				SetEndOfFile(hIndexFile);
				return 1;
			}
			while (1)
			{
				memset(&ReadBuff, NULL, sizeof(INDEXDATA));
				if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
					return 0;
				if (tmp_dw<1) break;
				SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);     //定位到开始写入的位置
				if (!WriteFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
					return 0;
				ReadAddr.QuadPart += sizeof(INDEXDATA);
				WriteAddr.QuadPart += sizeof(INDEXDATA);
				if (ReadAddr.QuadPart == DataFileSize.QuadPart)
				{
					SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
					SetEndOfFile(hIndexFile);                     //设置文件末地址
					return 1;
				}
				SetFilePointer(hIndexFile, ReadAddr.LowPart, &ReadAddr.HighPart, FILE_BEGIN);         //定位到开始读取的位置
			}
		}
	}

	return -1;
}

int OptimizeIndexFile()
{
	Queue Q;
	DWORD tmp_dw;
	QUEUE_DATATYPE elem;
	LARGE_INTEGER MovSize;

	memset(&Q, NULL, sizeof(Queue));
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));

	SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);                 //定位到起始位置
	SetEndOfFile(hIndexFile);            //清空索引文件中所有内容

	//广度优先遍历
	if (!EnQueue(&Q, AVL_ROOT))           //AVL树根入队
		return 0;
	while (Q.queuesize != 0)
	{
		DeQueue(&Q, &elem);
		if (elem->left)
		if (!EnQueue(&Q, elem->left))
			return 0;
		if (elem->right)
		if (!EnQueue(&Q, elem->right))
			return 0;
		if (!WriteFile(hIndexFile, &elem->data, sizeof(INDEXDATA), &tmp_dw, NULL))
			return 0;
	}

	return 1;
}

void ShowFile()
{
	FILE *file = NULL;
	LARGE_INTEGER MovSize;
	INDEXDATA IndexReadBuff;
	DWORD tmp_dw;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&IndexReadBuff, NULL, sizeof(INDEXDATA));

	if ((file = fopen("IndexForm.txt", "wt+")) == NULL)
	{
		print("创建临时文件失败.", RED);
		return;
	}

	fprintf(file, "索引文件结构:\n\n");
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);
	while (1)
	{
		if (!ReadFile(hIndexFile, &IndexReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
			break;
		if (tmp_dw<1) break;
		fprintf(file, "ASCII：%d\t\t", IndexReadBuff.ASCII);
		fprintf(file, "StartAddr:%d\t\t", IndexReadBuff.Start.QuadPart);
		fprintf(file, "WordCount:%d\n", IndexReadBuff.Count);
	}
	fclose(file);
	ShellExecute(NULL, _T("open"), _T("IndexForm.txt"), NULL, NULL, SW_SHOWNORMAL);
	return;
}