#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include "FileOper.h"
#include "global.h"
#include "AVL.h"
#include "GUI.h"

int InitFile()
{
	//检查文件的完整性,创建或打开文件.
	char choose;

	if (access(DataFilePath, 0) || access(IndexFilePath, 0))
	{
		//检测数据库文件或索引文件是否存在
		print("文件丢失,是否重建?(Y/N)", RED);
		while (1)
		{
			fflush(stdin);
			choose = _getch();
			if (choose == 'Y' || choose == 'y')
				break;
			else if (choose == 'N' || choose == 'n')
				exit(0);
		}
		if (!CreateDataFile()) return 0;
		if (!CreateIndexFile()) return 0;
		print("文件创建成功.", GREEN);
		return 1;
	}
	if (!OpenDataFile()) return 0;
	if (!OpenIndexFile()) return 0;
	return 1;
}

int InitIndex()
{
	DWORD tmp_dw;
	INDEXDATA ReadBuff;
	LARGE_INTEGER MovSize, CurrSize;

	AVL_ROOT = NULL;                     //AVL索引根节点指针置空
	memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));

	SetFilePointerEx(hIndexFile, MovSize, &CurrSize, FILE_BEGIN);        //将文件指针至于文件起始位置
	while (1)
	{
		if (!ReadFile(hIndexFile, (LPVOID)&ReadBuff, (DWORD)sizeof(INDEXDATA), &tmp_dw, NULL))
			return 0;
		if (tmp_dw<1) break;
		InsertIndexFromFile(&AVL_ROOT, &ReadBuff);          //将从文件内读出的索引插入到内存中
		memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	}

	return 1;
}