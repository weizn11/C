#include <time.h>
#include <stdio.h>
#include "GUI.h"

extern bool Thread_Switch;

Graph::Graph()
{
    loadimage(&image,"IMAGE","BACKGROUND",1000,600);
}

void Graph::Init_GUI()
{
    initgraph(1000,600);
    SetWindowText(GetHWnd(),"数据库课程设计_图书管理系统");
    loadimage(NULL,"IMAGE","BACKGROUND",1000,600);
    setlinecolor(RED);
    setlinestyle(PS_SOLID,2);
    //line(0,220,1000,220);
    line(250,0,250,600);
    line(725,0,725,600);
    line(725,450,1000,450);
    setlinestyle(PS_DOT);
	line(725,75,1000,75);
	line(725,150,1000,150);
	line(725,225,1000,225);
	line(725,300,1000,300);
	line(725,375,1000,375);
	line(725,525,1000,525);
	line(860,0,860,450);
    print_1("Esc键正常退出程序。",GREEN);
	settextcolor(BLACK);
	setbkmode(TRANSPARENT);
    outtextxy(760,30,"借阅图书");
	outtextxy(760,105,"归还图书");
	outtextxy(760,180,"添加图书");
	outtextxy(760,255,"删除图书");
	outtextxy(760,330,"添加学生");
	outtextxy(760,405,"删除学生");
	outtextxy(890,30,"导出图书表");
	outtextxy(890,105,"导出学生表");
	outtextxy(890,180,"导出借阅表");
	outtextxy(880,255,"即将超期图书");
	outtextxy(880,330,"查询借阅历史");
	outtextxy(895,405,"退出程序");
    CloseHandle(CreateThread(NULL,128,show_time,NULL,0x00010000,NULL));
}

void Graph::print_1(char *str,int color)
{
    char str1[1000],str2[31],*p=str;
    static int text1_y=2;

    memset(str1,NULL,sizeof(str1));
	strcat(str1,p);
	p=str1;
    if(text1_y>558)
    {
        cover(0,2,249,600);
        text1_y=2;
    }
    while(strlen(p)>30)
    {
        memset(str2,NULL,sizeof(str2));
        memcpy(str2,p,30);
        print_1(str2,color);
        p+=30;
    }
    settextcolor(color);
    setbkmode(TRANSPARENT);
    outtextxy(0,text1_y,p);
    text1_y+=16;
}

void Graph::print_2(char *str,int color)
{
    char str1[120],str2[60],*p=str;
    static int text2_y=2;

	memset(str1,NULL,sizeof(str1));
	strcat(str1,p);
	p=str1;
    if(text2_y>558)
    {
        cover(251,2,473,600);
        text2_y=2;
    }
    while(strlen(p)>58)
    {
        memset(str2,NULL,sizeof(str2));
        memcpy(str2,p,58);
        print_2(str2,color);
        p+=58;
    }
    settextcolor(color);
    setbkmode(TRANSPARENT);
    outtextxy(252,text2_y,p);
    text2_y+=16;
}

DWORD WINAPI Graph::show_time(LPVOID Parameter)
{
    struct tm TIME;
    IMAGE image,temp;
    time_t start=time(NULL),current=0,last=0;
    char current_time[50],last_time[50];
    loadimage(&image,"IMAGE","BACKGROUND",1000,600);
    while(1)
    {
        if(Thread_Switch) return -1;
        memset(current_time,NULL,sizeof(current_time));
        memset(last_time,NULL,sizeof(last_time));

        current=time(NULL);
        TIME=*localtime(&current);
        sprintf(current_time,"%d年%d月%d号 星期%s  %02d:%02d:%02d",TIME.tm_year+1900,TIME.tm_mon+1,\
			TIME.tm_mday,TIME.tm_wday==0?"天":TIME.tm_wday==1?"一":TIME.tm_wday==2?"二":TIME.tm_wday\
			==3?"三":TIME.tm_wday==4?"四":TIME.tm_wday==5?"五":"六",TIME.tm_hour,TIME.tm_min,TIME.tm_sec);
        SetWorkingImage(&image);
        getimage(&temp,726,527,274,90);
        SetWorkingImage(&temp);
        settextcolor(65484);
        setbkmode(TRANSPARENT);
        outtextxy(33,20,current_time);
        SetWorkingImage(NULL);
        putimage(726,527,274,90,&temp,0,0);

        last=current-start+57600;
        TIME=*localtime(&last);
        sprintf(last_time,"程序运行时长: %02d:%02d:%02d",TIME.tm_hour,TIME.tm_min,TIME.tm_sec);
        SetWorkingImage(&image);
        getimage(&temp,726,452,274,70);
        SetWorkingImage(&temp);
        settextcolor(65484);
        setbkmode(TRANSPARENT);
        outtextxy(48,30,last_time);
        SetWorkingImage(NULL);
        putimage(726,452,274,70,&temp,0,0);
        Sleep(10);
    }
    return 0;
}

inline void Graph::cover(int x,int y,int width,int height)
{
    IMAGE temp;
    SetWorkingImage(&image);
    getimage(&temp,x,y,width,height);
    SetWorkingImage(NULL);
    putimage(x,y,width,height,&temp,0,0);
}

int GetMouse()
{
	MOUSEMSG m;                 // 定义鼠标消息
	memset(&m,NULL,sizeof(MOUSEMSG));
	do
	{
		FlushMouseMsgBuffer();
		m=GetMouseMsg();          //获取一条鼠标消息
	}while(!m.mkLButton);         //若鼠标左键按下则退出循环
	
	if(m.x>725 && m.x<860)
	{
		if(m.y>0 && m.y<75)
			return 1;
		else if(m.y>75 && m.y<150)
			return 2;
		else if(m.y>150 && m.y<225)
			return 3;
		else if(m.y>225 && m.y<300)
			return 4;
		else if(m.y>300 && m.y<375)
			return 5;
		else if(m.y>375 && m.y<450)
			return 6;
	}
	else if(m.x>860 && m.x<1000)
	{
		if(m.y>0 && m.y<75)
			return 7;
		else if(m.y>75 && m.y<150)
			return 8;
		else if(m.y>150 && m.y<225)
			return 9;
		else if(m.y>225 && m.y<300)
			return 10;
		else if(m.y>300 && m.y<375)
			return 11;
		else if(m.y>375 && m.y<450)
			return 12;
	}
		
		return 0;
}