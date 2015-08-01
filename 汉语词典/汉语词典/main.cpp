#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#include "GUI.h"
#include "global.h"
#include "AVL.h"
#include "FileOper.h"
#include "init.h"
#include "menu.h"

int main(int argc, char *argv[])
{
	InitGUI();
	int choose, result;

	if (!InitFile())
	{
		print("初始化文件失败。", RED);
		_getch();
		return -1;
	}
	if (!InitIndex())
	{
		print("初始化索引失败。", RED);
		_getch();
		return -1;
	}

again:
	ShowAVLDepth();
	ShowCount();
	ShowFileSize();
	while (!(choose = GetMouse()));
	text2_y = 222;                  //控制台消息输出坐标初始化
	cover(251, 222, 473, 378);
	switch (choose)
	{
	case 1:
		if (!(result = Add_Word()))
		{
			print("添加词条失败。", RED);
		}
		else if (result == 1)
		{
			print("添加词条成功。", GREEN);
		}
		else
		{
			print("词库没有变动。", YELLOW);
		}
		break;
	case 2:
		Query();
		InitMenu();
		break;
	case 3:
		if (!OptimizeIndexFile())
		{
			print("优化索引文件失败,索引文件可能已经损坏!", RED);
		}
		else
		{
			print("优化索引文件成功.", GREEN);
		}
		print("是否查看索引文件结构?[Y/N]", GREEN);
		fflush(stdin);
		choose = _getch();
		if (choose == 'y' || choose == 'Y')
			ShowFile();
		text2_y = 222;                  //控制台消息输出坐标初始化
		cover(251, 222, 473, 378);
		break;
	}
	Sleep(500);           //防止鼠标连击
	goto again;
	return 0;
}