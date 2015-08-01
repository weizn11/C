#ifndef _FILEOPER_H_
#define _FILEOPER_H_

#include <stdio.h>
#include <stdlib.h>
#include "unicode.h"
#include "global.h"
#include "GUI.h"

int CreateDataFile()
{
	hDataFile = CreateFile(CharToTCHAR(DataFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDataFile == INVALID_HANDLE_VALUE)
	{
		print("创建数据库文件失败。", RED);
		return 0;
	}
	return 1;
}

int CreateIndexFile()
{
	hIndexFile = CreateFile(CharToTCHAR(IndexFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		print("创建索引文件失败。", RED);
		return 0;
	}
	return 1;
}

int OpenDataFile()
{
	hDataFile = CreateFile(CharToTCHAR(DataFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		printf("数据库文件打开失败。", RED);
		return 0;
	}
	return 1;
}

int OpenIndexFile()
{
	hIndexFile = CreateFile(CharToTCHAR(IndexFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		print("索引文件打开失败。", RED);
		return 0;
	}
	return 1;
}

int InsertDataToFile(WORD_INFO word, INDEXDATA index)
{
	DWORD tmp_dw;
	WORD_INFO temp;
	LARGE_INTEGER MovSize;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));

	MovSize.QuadPart = index.Start.QuadPart + sizeof(WORD_INFO)*(index.Count - 1);     //即将写入新词条的地址

	GetFileSizeEx(hDataFile, &DataFileSize);                                      //获取当前数据库文件的大小
	if (DataFileSize.QuadPart != MovSize.QuadPart)
	{
		//要插入的词条不在数据库文件的尾部
		do
		{
			DataFileSize.QuadPart -= sizeof(WORD_INFO);                         //将文件内部位置指针移至最后一个词条的开始处
			DataFileSize.LowPart = SetFilePointer(hDataFile, DataFileSize.LowPart, &DataFileSize.HighPart, FILE_BEGIN);
			ReadFile(hDataFile, &temp, sizeof(WORD_INFO), &tmp_dw, NULL);         //读出此词条
			WriteFile(hDataFile, &temp, sizeof(WORD_INFO), &tmp_dw, NULL);
		} while (DataFileSize.QuadPart != MovSize.QuadPart);
	}

	MovSize.LowPart = SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);      //文件内部位置指针移至将要追加的位置
	WriteFile(hDataFile, &word, sizeof(WORD_INFO), &tmp_dw, NULL);

	return 1;
}

int InsertIndexToFile(INDEXDATA IndexNode, int choose)
{
	//第二个参数1表示在文件末尾添加新索引,0表示修改某一索引值.
	int n = 0;
	DWORD tmp_dw;
	LARGE_INTEGER MovSize;
	INDEXDATA ReadBuff;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&ReadBuff, NULL, sizeof(INDEXDATA));

	if (choose == 0)
	{
		//表明此时是要修改索引文件中某索引
		SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);           //文件内部位置指针指向起始位置
		while (1)
		{
			if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
				return 0;
			if (tmp_dw<1) break;
			if (ReadBuff.ASCII == IndexNode.ASCII)
			{
				memset(&MovSize, NULL, sizeof(LARGE_INTEGER));                  //初始化位移变量
				MovSize.QuadPart = n*sizeof(INDEXDATA);                         //向后移动一个索引节点的长度
				SetFilePointerEx(hIndexFile, MovSize, NULL, FILE_BEGIN);         //第三个参数返回移动后的位置
				break;
			}
			n++;
			memset(&ReadBuff, NULL, sizeof(INDEXDATA));
		}
	}
	else
	{
		//将新的索引数据追加到索引文件的尾部
		MovSize.LowPart = SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_END);             //将文件内部指针移至文件末尾
	}
	if (!WriteFile(hIndexFile, (LPCVOID)&IndexNode, sizeof(INDEXDATA), &tmp_dw, NULL))         //将新的索引节点写入文件
		return 0;

	return 1;
}

#endif     //_FILEOPER_H_
