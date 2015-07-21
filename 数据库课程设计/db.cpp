#include <stdio.h>
#include <string.h>
#include "db.h"
#include "global.h"
#include "GUI.h"

MYSQL hMySQL;    //数据库连接句柄

Graph gui;

int create_database()
{
    int ret;
    char SQL[100],ErrorInfo[100],*ptmp=NULL;
    char *CreateBooksTable="create table Books("
												"BookNO VARCHAR(20) not null primary key,"
												"BookName VARCHAR(100) not null,"
												"Author VARCHAR(50),"
												"Press VARCHAR(50),"
												"MaxDays INT(4) not null,"
												"Borrowed INT(4));";

    char *CreateLendListTable="create table LendList("
												   	 "BookNO VARCHAR(20) not null primary key,"
												   	 "BookName VARCHAR(100) not null,"
											   	 	 "StudentNO VARCHAR(20) not null,"
											   	 	 "StudentName VARCHAR(50) not null,"
											   	 	 "LendDate datetime not null,"
													 "RetDate datetime not null);";
    char *CreateStudentsTable="create table Students("
													 "StudentNO VARCHAR(20) not null primary key,"
													 "StudentName VARCHAR(100) not null,"
													 "Class VARCHAR(100) not null,"
													 "MaxBorrow INT(4));";
	char *CreateHistoryListTable="create table HistoryList("
														   "BookNO VARCHAR(20) not null,"
														   "BookName VARCHAR(100) not null,"
														   "StudentNO VARCHAR(20) not null,"
														   "StudentName VARCHAR(50) not null,"
														   "LendDate datetime not null,"
														   "RetDate datetime not null);";

    //创建数据库
    memset(SQL,NULL,sizeof(SQL));
    sprintf(SQL,"create database %s;",DB_DATABASE);
    ret=mysql_query(&hMySQL,SQL);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Create database '%s' successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

    //更改数据库
    memset(SQL,NULL,sizeof(DB_DATABASE));
    sprintf(SQL,"use %s;",DB_DATABASE);
    ret=mysql_query(&hMySQL,SQL);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Change database successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

    //创建学生表
    ret=mysql_query(&hMySQL,CreateStudentsTable);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Create 'Students' table successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

    //创建图书表
    ret=mysql_query(&hMySQL,CreateBooksTable);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Create 'Books' table successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

    //创建借阅表
    ret=mysql_query(&hMySQL,CreateLendListTable);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Create 'LendList' table successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

	//创建借阅历史表
    ret=mysql_query(&hMySQL,CreateHistoryListTable);
    if(ret!=0)
    {
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);
        return -1;
    }
    else
    {
        memset(SQL,NULL,sizeof(SQL));
        sprintf(SQL,"Create 'HistoryList' table successfully!",DB_DATABASE);
        gui.print_1(SQL,GREEN);
    }

    return 0;
}

int connect_to_database()
{
    int ret;
    char SQL[100],ErrorInfo[100],*ptmp=NULL;

    memset(SQL,NULL,sizeof(SQL));

    mysql_init(&hMySQL);          //初始化连接句柄
    if(!mysql_real_connect(&hMySQL, DB_HOST, DB_USERNAME, DB_PASSWORD,NULL, 0, NULL, CLIENT_FOUND_ROWS))
    {
        gui.print_1("Database connect failed!",RED);
        return -1;
    }
    gui.print_1("Database connect successfully!",GREEN);

	//ret=mysql_query(&hMySQL,"drop database library;");

    sprintf(SQL,"use %s;",DB_DATABASE);
    ret=mysql_query(&hMySQL,SQL);

    if(ret!=0)
    {
        //查询失败
        memset(ErrorInfo,NULL,sizeof(ErrorInfo));
        strcpy(ErrorInfo,mysql_error(&hMySQL));
        gui.print_1(ErrorInfo,RED);

        ptmp=strstr(ErrorInfo,"Unknown database");
        if(ptmp==NULL) return -1;
        if(create_database()!=0) return -1;
    }

    return 0;
}
