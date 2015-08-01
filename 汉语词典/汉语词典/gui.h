#ifndef _GUI_H_
#define _GUI_H_

#include <graphics.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>

#include "unicode.h"
#include "global.h"

int InitGUI();
int InitMenu();
void print(char *str, int color);
void cover(int x, int y, int width, int height);
void ShowAVLDepth();
void ShowAuthor();
void ShowCount();
void ShowFileSize();

IMAGE image;
CRITICAL_SECTION cs_image;                    //声明临界区对象

int InitGUI()
{
	InitializeCriticalSection(&cs_image);     //初始化临界区
	initgraph(1000, 600);
	SetWindowText(GetHWnd(), _T("汉语词典"));
	loadimage(&image, _T("IMAGE"), _T("BACKGROUND"), 1000, 600);    //初始化背景图片       
	SetWorkingImage(NULL);
	putimage(0, 0, 1000, 600, &image, 0, 0);
	setlinecolor(RED);
	setlinestyle(PS_SOLID, 2);
	line(0, 220, 1000, 220);
	line(0, 345, 250, 345);
	line(0, 470, 250, 470);
	line(250, 220, 250, 600);
	line(725, 220, 725, 600);
	line(725, 408, 1000, 408);
	setlinestyle(PS_DOT);
	line(725, 312, 1000, 312);
	line(725, 504, 1000, 504);
	InitMenu();
	ShowAuthor();
	return 1;
}

int InitMenu()
{
	cover(40, 250, 150, 50);
	cover(40, 370, 150, 50);
	cover(30, 500, 190, 50);

	settextcolor(GREEN);
	setbkmode(TRANSPARENT);
	settextstyle(15, 15, NULL);
	outtextxy(55, 270, _T("添加词条"));
	outtextxy(55, 395, _T("查找词条"));
	outtextxy(30, 525, _T("优化索引文件"));
	return 1;
}

int Depth(AVL *root);
void ShowAVLDepth()
{
	char str[100];
	int depth = 0;

	cover(726, 221, 273, 80);
	settextstyle(0, 0, NULL);
	settextcolor(GREEN);
	setbkmode(TRANSPARENT);
	memset(str, NULL, sizeof(str));
	if (AVL_ROOT)
		depth = Depth(AVL_ROOT->left);
	else
		depth = 0;
	sprintf(str, "AVL左子树深度:%d", depth);
	outtextxy(780, 240, CharToTCHAR(str));
	memset(str, NULL, sizeof(str));
	if (AVL_ROOT)
		depth = Depth(AVL_ROOT->right);
	else
		depth = 0;
	sprintf(str, "AVL右子树深度:%d", depth);
	outtextxy(780, 275, CharToTCHAR(str));

	return;
}

int NodeCount(AVL *root);
int WordCount(AVL *root);
void ShowCount()
{
	char str[100];

	cover(726, 314, 273, 80);
	memset(str, NULL, sizeof(str));
	settextstyle(0, 0, NULL);
	settextcolor(GREEN);
	setbkmode(TRANSPARENT);
	memset(str, NULL, sizeof(str));
	sprintf(str, "AVL节点个数:%d", NodeCount(AVL_ROOT));
	outtextxy(785, 330, CharToTCHAR(str));
	memset(str, NULL, sizeof(str));
	sprintf(str, "已收录词条个数:%d", WordCount(AVL_ROOT));
	outtextxy(780, 365, CharToTCHAR(str));

	return;
}

void ShowFileSize()
{
	char str[50];

	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));
	cover(726, 410, 273, 80);
	settextstyle(0, 0, NULL);
	settextcolor(GREEN);
	setbkmode(TRANSPARENT);
	GetFileSizeEx(hIndexFile, &DataFileSize);             //获取索引文件大小
	memset(str, NULL, sizeof(str));
	sprintf(str, "索引文件大小:%d字节", DataFileSize.QuadPart);
	outtextxy(775, 425, CharToTCHAR(str));
	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));
	GetFileSizeEx(hDataFile, &DataFileSize);             //获取索引文件大小
	memset(str, NULL, sizeof(str));
	sprintf(str, "词库文件大小:%d字节", DataFileSize.QuadPart);
	outtextxy(775, 465, CharToTCHAR(str));

	return;
}

void ShowAuthor()
{
	settextstyle(0, 0, NULL);
	settextcolor(GREEN);
	setbkmode(TRANSPARENT);
	outtextxy(780, 540, _T("By:计科12-1班  魏子楠"));
}

int GetMouse()
{
	MOUSEMSG m;                 // 定义鼠标消息
	memset(&m, NULL, sizeof(MOUSEMSG));
	do
	{
		FlushMouseMsgBuffer();
		m = GetMouseMsg();          //获取一条鼠标消息
	} while (!m.mkLButton);         //若鼠标左键按下则退出循环

	if (m.x>0 && m.x<250)
	if (m.y>220 && m.y<345)
		return 1;
	else if (m.y>345 && m.y<470)
		return 2;
	else if (m.y>470 && m.y<600)
		return 3;

	return 0;
}

static int text2_y = 222;                 //控制台输出消息坐标
void print(char *str, int color)
{
	char str1[120], str2[60], *p = str;

	memset(str1, NULL, sizeof(str1));
	strcat(str1, p);
	p = str1;
	if (text2_y>558)
	{
		cover(251, 222, 473, 378);
		text2_y = 222;
	}
	while (strlen(p)>58)
	{
		memset(str2, NULL, sizeof(str2));
		memcpy(str2, p, 58);
		print(str2, color);
		p += 58;
	}
	settextstyle(0, 0, NULL);
	settextcolor(color);
	setbkmode(TRANSPARENT);
	outtextxy(252, text2_y, CharToTCHAR(p));
	text2_y += 16;
}

void cover(int x, int y, int width, int height)
{
	IMAGE temp;
	EnterCriticalSection(&cs_image);              //进入临界区
	SetWorkingImage(&image);
	getimage(&temp, x, y, width, height);
	SetWorkingImage(NULL);
	LeaveCriticalSection(&cs_image);              //离开临界区
	putimage(x, y, &temp);
}

#endif    //_GUI_H_
