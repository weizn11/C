#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <odbcss.h>

#define MAXBUFLEN 255

SQLHENV henv = SQL_NULL_HENV;
SQLHDBC hdbc = SQL_NULL_HDBC;
SQLHSTMT hstmt = SQL_NULL_HSTMT;

int Connect_SQLServer()
{
    RETCODE retcode;
	
	SQLCHAR ConnStrIn[MAXBUFLEN] = "DRIVER={SQL Server};SERVER=210.44.159.15,14;UID=IIS;PWD=305859616;DATABASE=master;";
    //1.连接数据源
    //1.环境句柄 
    retcode=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE ,&henv); 
    retcode=SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,SQL_IS_INTEGER);
    //2.连接句柄
    retcode=SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    retcode=SQLDriverConnect(hdbc,NULL,ConnStrIn,SQL_NTS,NULL,NULL,NULL,SQL_DRIVER_NOPROMPT);
    //判断连接是否成功
    if((retcode!=SQL_SUCCESS)&&(retcode!=SQL_SUCCESS_WITH_INFO))
        return 0;
	return 1;
}

int ChangeDB(char *DBN)
{
	RETCODE retcode;
	SQLHSTMT  hstmt;
	char SQL_CMD[100];
	
	memset(SQL_CMD,NULL,sizeof(SQL_CMD));
    sprintf(SQL_CMD,"use %s;",DBN);
	retcode= SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if(retcode!=SQL_SUCCESS && retcode!=SQL_SUCCESS_WITH_INFO)
		return 0;
	SQLFreeStmt(hstmt,SQL_CLOSE); 
	retcode=SQLExecDirect(hstmt,(unsigned char *)SQL_CMD,SQL_NTS);
	if(retcode!=SQL_SUCCESS && retcode!=SQL_SUCCESS_WITH_INFO)
		return 0;
	
	return 1;
}

int QueryDB(SQLHSTMT *hstmt,char *SQL_CMD)
{
	RETCODE retcode;
	

	retcode= SQLAllocHandle(SQL_HANDLE_STMT, hdbc, hstmt);
	if(retcode!=SQL_SUCCESS && retcode!=SQL_SUCCESS_WITH_INFO)
		return 0;
	SQLFreeStmt(*hstmt,SQL_CLOSE); 
	retcode=SQLExecDirect(*hstmt,(unsigned char *)SQL_CMD,SQL_NTS);
	if(retcode!=SQL_SUCCESS && retcode!=SQL_SUCCESS_WITH_INFO)
		return 0;
	
	return 1;
}

int show_results(SQLHSTMT hstmt)
{
	RETCODE retcode;
	SQLINTEGER RowCount;
	char data[100];
    SQLCHAR   buff[50];
    SQLINTEGER  len;
	int flag=0;

    retcode=SQLRowCount(hstmt,&RowCount);
	if(retcode!=SQL_SUCCESS && retcode!=SQL_SUCCESS_WITH_INFO)
		return 0;

	while(1)
	{
		memset(data, 0, sizeof(data));
		retcode = SQLFetch(hstmt);
		if (retcode==SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			memset(buff,NULL,sizeof(buff));
			SQLGetData(hstmt,(UWORD)2,SQL_C_CHAR,buff,sizeof(buff)-1, &len);  //第二个参数表示列

// 			if(!strncmp((char *)buff,"3",1) || !strncmp((char *)buff,"2",1) \
// 				|| !strncmp((char *)buff,"1",1))
// 				flag=1;
// 			else
// 				flag=0;
			strcat(data,(char *)buff);

			strcat(data, "\t");
			memset(buff,NULL,sizeof(buff));
			SQLGetData(hstmt,(UWORD)3,SQL_C_CHAR,buff,sizeof(buff)-1,&len);
			strcat(data,(char *)buff);

			strcat(data, "\t");
			memset(buff,NULL,sizeof(buff));
			SQLGetData(hstmt,(UWORD)4,SQL_C_CHAR,buff,sizeof(buff)-1,&len);
// 			if(!strcmp((char *)buff,"17") && flag==1)
// 				flag=1;
// 			else
// 				flag=0;
			strcat(data,(char *)buff);

			strcat(data, "\t");
			memset(buff,NULL,sizeof(buff));
			SQLGetData(hstmt,(UWORD)5,SQL_C_CHAR,buff,sizeof(buff)-1,&len);
			if(!strcmp((char *)buff,"0"))
			    strcat(data,"男");
			else
				strcat(data,"女");

			strcat(data, "\t");
			memset(buff,NULL,sizeof(buff));
			SQLGetData(hstmt,(UWORD)7,SQL_C_CHAR,buff,sizeof(buff)-1,&len);
			strcat(data,(char *)buff);
		}
		else
			return 1;
//		if(flag)
		    printf("%s\n",data);
	}
}

int main()
{
    SQLHSTMT hstmt;
    char buff[100];
    char query[100];
	char choose[10];
	int tmp;
    int i,j;
	
    system("color a");
    if(!Connect_SQLServer())
    {
        printf("数据库连接失败\n");
        getch();
        return -1;
    }
    printf("数据库连接成功\n");
	printf("请输入检索方式:\n1.QQ号\t2.姓名\t3.群号\n");
	fflush(stdin);
	if(scanf("%d",&tmp)!=1) return -1;
	memset(choose,NULL,sizeof(choose));
	switch (tmp)
	{
	case 1:
		strcat(choose,"QQNum");
		break;
	case 2:
		strcat(choose,"Nick");
		break;
	case 3:
		strcat(choose,"QunNum");
		break;
	default:
		return -1;
	}

	
    printf("请输入要查询的内容:");
    memset(query,NULL,sizeof(query));
    fflush(stdin);
    gets(query);
	
	printf("QQNum\t\tName\tAge\tSex\tQunNum\n");
    for(i=1,j=1; i<=11; i++)
    {
		//printf("正在查询第%d个数据库。\n",i);
        memset(buff,NULL,sizeof(buff));
        sprintf(buff,"GroupData%d",i);
        if(!ChangeDB(buff))
            break;
        for(;; j++)
        {
			//printf("正在查询第%d个表。\n",j);
			memset(buff,NULL,sizeof(buff));
			sprintf(buff,"select * from Group%d where %s like '%%%s%%';",j,choose,query);
            if(!QueryDB(&hstmt,buff))
				break;
			
            show_results(hstmt);
        }
    }
	printf("查询完成。\n");
	system("pause");
	
    return 0;
}

