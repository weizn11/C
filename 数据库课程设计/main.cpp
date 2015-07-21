#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <conio.h>
#include <time.h>
#include "global.h"
#include "unicode.h"
#include "GUI.h"
#include "db.h"

#define SERVICE_PORT 7777
#define MAX_SIZE 1024

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"libmysql")

bool Thread_Switch=FALSE;
extern Graph gui;
extern MYSQL hMySQL;

void exit()
{
	Thread_Switch=TRUE;
	Sleep(1000);      //等待所有线程结束
    exit(0);
}

DWORD WINAPI Listen_Keyboard(LPVOID)
{
    while(1)
    {
        fflush(stdin);
        if(getch()==27)
        {
			exit();
        }
    }
    return 0;
}

int lend_book()
{
	char *SQL_LendBook="insert into LendList values('%s','%s','%s','%s','%s','%s');";
	char *SQL_QueryBorrowed="select Borrowed from Books where BookNO='%s';";
	char *SQL_QueryBookName="select BookName from Books where BookNO='%s';";
	char *SQL_QueryStudentName="select StudentName from Students where StudentNO='%s';";
	char *SQL_UpdateBorrowed="update Books set Borrowed=1 where BookNO='%s';";
	char *SQL_QueryBorrowedCount="select StudentNO from LendList where StudentNO='%s';";
	char *SQL_QueryMaxBorrow="select MaxBorrow from Students where StudentNO='%s';";
	char *SQL_QueryMaxDays="select MaxDays from Books where BookNO='%s';";
	char *SQL_AddHistoryList="insert into HistoryList values('%s','%s','%s','%s','%s','%s')";
	char BookNOBuffer[MAX_SIZE],StudentNOBuffer[MAX_SIZE];
	char BookNameBuffer[MAX_SIZE],StudentNameBuffer[MAX_SIZE];
	char SQL[MAX_SIZE],Buffer[MAX_SIZE];
	char current_time[50],ret_time[50];

	int ret,row,borrowed,MaxBorrow;
	long int current,rettime,MaxDays;
	MYSQL_ROW result_row;	/*按行返回的查询信息*/
	MYSQL_RES *pResults;
	struct tm TIME;

	memset(BookNOBuffer,NULL,sizeof(BookNOBuffer));
	memset(StudentNOBuffer,NULL,sizeof(StudentNOBuffer));
	memset(BookNameBuffer,NULL,sizeof(BookNameBuffer));
	memset(StudentNameBuffer,NULL,sizeof(StudentNameBuffer));
	memset(SQL,NULL,sizeof(SQL));
	memset(Buffer,NULL,sizeof(Buffer));
	memset(current_time,NULL,sizeof(current_time));

	if(InputBox(BookNOBuffer,sizeof(BookNOBuffer)-1,"请输入图书编号","借阅图书",NULL,100,100,false)==false)
		return -1;
	if(InputBox(StudentNOBuffer,sizeof(StudentNOBuffer)-1,"请输入学生学号","借阅图书",NULL,100,100,false)==false)
		return -1;

	//查询图书名
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryBookName,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		row=mysql_num_rows(pResults);    //取得行数
		if(row<1)
		{
			mysql_free_result(pResults);
			return 3;				//不存在此图书
		}
		result_row=mysql_fetch_row(pResults);
		strcpy(BookNameBuffer,result_row[0]);
		mysql_free_result(pResults);
	}

	//查询图书借阅情况
	sprintf(SQL,SQL_QueryBorrowed,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		row=mysql_num_rows(pResults);    //取得行数
		if(row<1)
		{
			mysql_free_result(pResults);
			return 3;				//不存在此图书
		}
		result_row=mysql_fetch_row(pResults);
		borrowed=atoi(result_row[0]);
		if(borrowed!=0)
		{
			mysql_free_result(pResults);
			sprintf(Buffer,"《%s》已被借出",BookNameBuffer);
			gui.print_2(Buffer,YELLOW);
			return 1;	//图书已被借阅
		}
		mysql_free_result(pResults);
	}

	//查询学生姓名
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryStudentName,StudentNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		row=mysql_num_rows(pResults);    //取得行数
		if(row<1)
		{
			mysql_free_result(pResults);
			return 4;				//不存在此学生
		}
		result_row=mysql_fetch_row(pResults);
		strcpy(StudentNameBuffer,result_row[0]);
		mysql_free_result(pResults);
	}

	//查询此学生已借阅数
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryBorrowedCount,StudentNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		borrowed=mysql_num_rows(pResults);    //取得已借阅数
		mysql_free_result(pResults);
	}

	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryMaxBorrow,StudentNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}

	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		memset(SQL,NULL,sizeof(SQL));
		result_row=mysql_fetch_row(pResults);
		MaxBorrow=atoi(result_row[0]);		//取得最大借阅数
		mysql_free_result(pResults);
	}

	if(MaxBorrow<=borrowed)
	{
		memset(SQL,NULL,sizeof(SQL));
		sprintf(SQL,"该学生已达到借阅上限%d本",borrowed);
		gui.print_2(SQL,YELLOW);
		return -1;
	}

	//查询最大借阅天数
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryMaxDays,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		result_row=mysql_fetch_row(pResults);
		MaxDays=atoi(result_row[0]);      //获取图书的最大借阅天数
		mysql_free_result(pResults);
	}

	//插入借阅表和借阅历史表
	memset(SQL,NULL,sizeof(SQL));
	current=time(NULL);
	TIME=*localtime(&current);
	sprintf(current_time,"%d-%d-%d %02d:%02d:%02d",TIME.tm_year+1900,TIME.tm_mon+1,\
		TIME.tm_mday,TIME.tm_hour,TIME.tm_min,TIME.tm_sec);
	rettime=current+MaxDays*24*60*60;
	TIME=*localtime(&rettime);
	sprintf(ret_time,"%d-%d-%d %02d:%02d:%02d",TIME.tm_year+1900,TIME.tm_mon+1,\
		TIME.tm_mday,TIME.tm_hour,TIME.tm_min,TIME.tm_sec);
	sprintf(SQL,SQL_LendBook,BookNOBuffer,BookNameBuffer,StudentNOBuffer,StudentNameBuffer,current_time,ret_time);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_AddHistoryList,BookNOBuffer,BookNameBuffer,StudentNOBuffer,StudentNameBuffer,current_time,ret_time);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 2;
	}

	//更新图书借阅情况
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_UpdateBorrowed,BookNOBuffer);
	mysql_query(&hMySQL,SQL);

	memset(Buffer,NULL,sizeof(Buffer));
	sprintf(Buffer,"《%s》被'%s'借阅成功",BookNameBuffer,StudentNameBuffer);
	gui.print_2(Buffer,GREEN);

	return 0;
}

int add_book()
{
	char *SQL_AddBook="insert into Books values('%s','%s','%s','%s',%d,0);";
	char FilePath[MAX_SIZE];
	char BookNOBuffer[MAX_SIZE],BookNameBuffer[MAX_SIZE],BookAuthorBuffer[MAX_SIZE],Press[MAX_SIZE],MaxDays[10];
	char SQL[MAX_SIZE],Buffer[MAX_SIZE];
	int ret;
	FILE *file=NULL;

	memset(FilePath,NULL,sizeof(FilePath));

	if(InputBox(FilePath,sizeof(FilePath)-1,"请输入导入文件名","添加图书",NULL,100,100,false)==false)
		return -1;

	if((file=fopen(FilePath,"rt"))==NULL)
	{
		gui.print_2("文件打开失败",RED);
		return -1;
	}

	while(!feof(file))
	{
	memset(BookAuthorBuffer,NULL,sizeof(BookAuthorBuffer));
	memset(BookNameBuffer,NULL,sizeof(BookNameBuffer));
	memset(BookNOBuffer,NULL,sizeof(BookNOBuffer));
	memset(SQL,NULL,sizeof(SQL));
	memset(Buffer,NULL,sizeof(Buffer));
	memset(Press,NULL,sizeof(Press));
	memset(MaxDays,NULL,sizeof(MaxDays));

	fscanf(file,"%s\t%s\t%s\t%s\t%s",BookNOBuffer,BookNameBuffer,BookAuthorBuffer,Press,MaxDays);
	if(strlen(BookNOBuffer)+strlen(BookNameBuffer)<2) continue;

	sprintf(SQL,SQL_AddBook,BookNOBuffer,BookNameBuffer,BookAuthorBuffer,Press,atoi(MaxDays));
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcat(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		continue;
	}

	memset(Buffer,NULL,sizeof(Buffer));
	sprintf(Buffer,"添加图书'%s 《%s》 %s %s 借阅期限:%s天'",\
		BookNOBuffer,BookNameBuffer,BookAuthorBuffer,Press,MaxDays);
	gui.print_2(Buffer,GREEN);
	}

	return 0;
}

int add_student()
{
	char *SQL_AddStudent="insert into Students values('%s','%s','%s',%d);";
	char StudentNOBuffer[MAX_SIZE],StudentNameBuffer[MAX_SIZE],StudentClassBuffer[MAX_SIZE];
	char Buffer[MAX_SIZE],SQL[MAX_SIZE],FilePath[MAX_SIZE],MaxBorrow[10];
	int ret;
	FILE *file;

	memset(FilePath,NULL,sizeof(FilePath));

	if(InputBox(FilePath,sizeof(FilePath)-1,"请输入导入文件名","添加学生",NULL,100,100,false)==false)
		return -1;

	if((file=fopen(FilePath,"rt"))==NULL)
	{
		gui.print_2("文件打开失败",RED);
		return -1;
	}

	while(!feof(file))
	{
		memset(StudentClassBuffer,NULL,sizeof(StudentClassBuffer));
	memset(StudentNameBuffer,NULL,sizeof(StudentNameBuffer));
	memset(StudentNOBuffer,NULL,sizeof(StudentNOBuffer));
	memset(Buffer,NULL,sizeof(Buffer));
	memset(SQL,NULL,sizeof(SQL));
	memset(MaxBorrow,NULL,sizeof(MaxBorrow));

	fscanf(file,"%s\t%s\t%s\t%s",StudentNOBuffer,StudentNameBuffer,StudentClassBuffer,MaxBorrow);
	sprintf(SQL,SQL_AddStudent,StudentNOBuffer,StudentNameBuffer,StudentClassBuffer,atoi(MaxBorrow));
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		continue;
	}

	sprintf(Buffer,"已添加学生'%s %s %s 最大借阅数:%s'",StudentNOBuffer,StudentNameBuffer,StudentClassBuffer,MaxBorrow);
	gui.print_2(Buffer,GREEN);
	}

	return 0;
}

int ret_book()
{
	char *SQL_DeleteLendInfo="delete from LendList where BookNO='%s';";
	char *SQL_UpdateBorrowed="update Books set Borrowed='0' where BookNO='%s';";
	char *SQL_QueryBookName="select BookName from Books where BookNO='%s';";
	char BookNOBuffer[MAX_SIZE],BookNameBuffer[MAX_SIZE],SQL[MAX_SIZE],Buffer[MAX_SIZE];
	int ret,row;
	MYSQL_RES *pResults;
	MYSQL_ROW ResultRows;

	memset(BookNOBuffer,NULL,sizeof(BookNOBuffer));
	memset(SQL,NULL,sizeof(SQL));
	memset(Buffer,NULL,sizeof(Buffer));

	if(InputBox(BookNOBuffer,sizeof(BookNOBuffer)-1,"请输入图书编号","归还图书",NULL,100,100,false)==false)
		return -1;

	//查询书名
	sprintf(SQL,SQL_QueryBookName,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}

	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		row=mysql_num_rows(pResults);
		if(row<1)
		{
			mysql_free_result(pResults);
			return 2;             //不存在此书
		}
		ResultRows=mysql_fetch_row(pResults);
		strcpy(BookNameBuffer,ResultRows[0]);
		mysql_free_result(pResults);
	}

	//删除借阅条目
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_DeleteLendInfo,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		strcpy(Buffer,mysql_error(&hMySQL));
		gui.print_1(Buffer,RED);
		return 1;
	}
	row=mysql_affected_rows(&hMySQL);
	if(row==0)
	{
		sprintf(Buffer,"图书《%s》未借出",BookNameBuffer);
		gui.print_2(Buffer,YELLOW);
		return 3;
	}

	//更新借阅状态
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_UpdateBorrowed,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}

	sprintf(Buffer,"《%s》已成功归还",BookNameBuffer);
	gui.print_2(Buffer,GREEN);

	return 0;
}

int del_book()
{
	char *SQL_DeleteBook="delete from Books where BookNO='%s';";
	char *SQL_DeleteLendInfo="delete from LendList where BookNO='%s';";
	char BookNOBuffer[MAX_SIZE],Buffer[MAX_SIZE],SQL[MAX_SIZE];
	int ret,row;

	memset(BookNOBuffer,NULL,sizeof(BookNOBuffer));
	memset(Buffer,NULL,sizeof(Buffer));
	memset(SQL,NULL,sizeof(SQL));
	
	if(InputBox(BookNOBuffer,sizeof(BookNOBuffer)-1,"请输入图书编号","删除图书",NULL,100,100,false)==false)
		return -1;

	//删除图书
	sprintf(SQL,SQL_DeleteBook,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}
	row=mysql_affected_rows(&hMySQL);
	if(row==0)
	{
		gui.print_2("不存在此编号的图书",YELLOW);
		return -1;
	}

	//删除借阅关系
	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_DeleteLendInfo,BookNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}

	sprintf(Buffer,"编号为'%s'的图书已被删除",BookNOBuffer);
	gui.print_2(Buffer,GREEN);

	return 0;
}

int del_student()
{
	char *SQL_DeleteStudent="delete from Students where StudentNO='%s';";
	char StudentNOBuffer[MAX_SIZE],SQL[MAX_SIZE];
	int ret;

	memset(StudentNOBuffer,NULL,sizeof(StudentNOBuffer));
	memset(SQL,NULL,sizeof(SQL));

	if(InputBox(StudentNOBuffer,sizeof(StudentNOBuffer)-1,"请输入学生学号","删除学生",NULL,100,100,false)==false)
		return -1;

	sprintf(SQL,SQL_DeleteStudent,StudentNOBuffer);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}

	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,"删除编号为'%s'学生成功",StudentNOBuffer);
	gui.print_2(SQL,GREEN);

	return 0;
}

int import_table(char *tablename)
{
	char *SQL_QueryTable="select * from %s;";
	char Buffer[MAX_SIZE],SQL[MAX_SIZE],FilePath[MAX_SIZE];
	FILE *file;
	int ret,rows,colums,i,j;
	MYSQL_RES *pResults;
	MYSQL_ROW RowResults;

	memset(FilePath,NULL,sizeof(FilePath));
	
	sprintf(FilePath,"%s.txt",tablename);
	if((file=fopen(FilePath,"wt"))==NULL)
	{
		gui.print_2("创建文件失败",RED);
		return -1;
	}

	memset(SQL,NULL,sizeof(SQL));
	sprintf(SQL,SQL_QueryTable,tablename);
	gui.print_1(SQL,YELLOW);
	ret=mysql_query(&hMySQL,SQL);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		rows=mysql_num_rows(pResults);
		colums=mysql_num_fields(pResults);
		for(i=0;i<rows;i++)
		{
			RowResults=mysql_fetch_row(pResults);
			for(j=0;j<colums;j++)
			{
				memset(Buffer,NULL,sizeof(Buffer));
				strcpy(Buffer,RowResults[j]);
				fprintf(file,"%s\t",Buffer);
			}
			fprintf(file,"\n");
			fflush(file);
		}
	}
	fclose(file);

	memset(Buffer,NULL,sizeof(Buffer));
	sprintf(Buffer,"已成功导出'%s'表",tablename);
	gui.print_2(Buffer,GREEN);

	ShellExecute(NULL,"open",FilePath,NULL,NULL,SW_SHOWNORMAL);

	return 0;
}

int import_will_due()
{
	char *SQL_QueryCount="select BookNO from LendList;";
	char *SQL_QueryDevTime="select unix_timestamp(RetDate)-unix_timestamp(LendDate),StudentNO,StudentName,BookName"
						   " from LendList limit %d,%d;";
	char Buffer[MAX_SIZE],SQL[MAX_SIZE];
	int ret,i,rows;
	long int time;
	MYSQL_RES *pResults;
	MYSQL_ROW RowResults;
	FILE *file=NULL;

	if((file=fopen("willdue.txt","wt"))==NULL)
	{
		gui.print_2("创建文件失败",RED);
		return -1;
	}

	ret=mysql_query(&hMySQL,SQL_QueryCount);
	gui.print_1(SQL_QueryCount,YELLOW);
	if(ret!=0)
	{
		gui.print_1((char *)mysql_error(&hMySQL),RED);
		return 1;
	}
	pResults=mysql_store_result(&hMySQL);
	if(pResults)
	{
		rows=mysql_num_rows(pResults);
		mysql_free_result(pResults);
	}

	for(i=0;i<rows;i++)
	{
		memset(SQL,NULL,sizeof(SQL));
		sprintf(SQL,SQL_QueryDevTime,i,i+1);
		gui.print_1(SQL,YELLOW);
		ret=mysql_query(&hMySQL,SQL);
		if(ret!=0)
		{
			gui.print_1((char *)mysql_error(&hMySQL),RED);
			return 1;
		}
		pResults=mysql_store_result(&hMySQL);
		RowResults=mysql_fetch_row(pResults);
		time=atol(RowResults[0]);      //获取时间差
		if(time<0 || time>5*24*60*60)
		{
			mysql_free_result(pResults);
			continue;
		}
		memset(Buffer,NULL,sizeof(Buffer));
		fprintf(file,"学号:%s\t姓名:%s\t借阅图书:%s\t还剩%d天\n",RowResults[1],RowResults[2],RowResults[3],time/60/60/24);
		fflush(file);
		mysql_free_result(pResults);
	}
	fclose(file);

	ShellExecute(NULL,"open","willdue.txt",NULL,NULL,SW_SHOWNORMAL);

	return 0;
}

int main(int argc,char *argv[])
{
	int choose;
    Graph GUI;

    CloseHandle(CreateThread(NULL,0,Listen_Keyboard,NULL,0,NULL));   //监听键盘线程

    GUI.Init_GUI();         //初始化操作界面
	connect_to_database();

	while(1)
	{
		while(!(choose=GetMouse()));

		switch (choose)
		{
		case 1:
			//借阅图书
			switch(lend_book())
			{
			case 2:
				gui.print_2("数据库操作失败",RED);
				break;
			case 3:
				gui.print_2("不存在此图书",YELLOW);
				break;
			case 4:
				gui.print_2("不存在此学生",YELLOW);
				break;
			}
			break;
		case 2:
			//归还图书
			switch (ret_book())
			{
			case 1:
				gui.print_2("数据库操作出错",RED);
				break;
			case 2:
				gui.print_2("不存在此编号图书",YELLOW);
				break;
			}
			break;
		case 3:
			//添加图书
			switch(add_book())
			{
			case 1:
				gui.print_2("数据库操作失败",RED);
				break;
			case 0:
				gui.print_2("添加图书完毕",GREEN);
				break;
			}
			break;
		case 4:
			//删除图书
			switch (del_book())
			{
			case 1:
				gui.print_2("数据库操作出错",RED);
				break;
			}
			break;
		case 5:
			//添加学生
			switch (add_student())
			{
			case 1:
				gui.print_2("数据库操作出错",RED);
				break;
			case 0:
				gui.print_2("添加学生完毕",GREEN);
				break;
			}
			break;
		case 6:
			//删除学生
			switch (del_student())
			{
			case 1:
				gui.print_2("数据库操作出错",RED);
				break;
			}
			break;
		case 7:
			//导出图书表
			import_table("Books");
			break;
		case 8:
			//导出学生表
			import_table("Students");
			break;
		case 9:
			//导出借阅表
			import_table("LendList");
			break;
		case 10:
			//导出即将超期图书
			import_will_due();
			break;
		case 11:
			//查询借阅历史
			import_table("HistoryList");
			break;
		case 12:
			exit();
		default:
			break;
		}
	}
    
    return 0;
}











