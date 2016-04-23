#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <pthread.h>

#include "db.h"
#include "../concurrent/async.h"

MYSQL hMySQLServerConnection;
pthread_mutex_t MutexMySQLConn;

int db_create_database()
{
    int ret;
    char sql[SQL_MAX_SIZE];

    //create database
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_CREATE_DATABASE,DB_DATABASE);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create database successful.");

    //change database
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_CHANGE_USE_DB,DB_DATABASE);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }

    //set UTF-8
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_SET_CHARACTER,DB_DATABASE);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    printf("set UTF-8 scuuessful.\n");

    //create users table
    memset(sql,NULL,sizeof(sql));
    strcat(sql,SQL_CREATE_TABLE_USERS);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create 'users' table successful.");

    //create friendship table
    memset(sql,NULL,sizeof(sql));
    strcat(sql,SQL_CREATE_TABLE_FRIENDSHIP);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create 'friendship' table successful.");

    //create leavemessages table
    memset(sql,NULL,sizeof(sql));
    strcat(sql,SQL_CREATE_TABLE_LEAVEMESSAGES);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create 'leavemessages' table successful.");

    //create groups table
    memset(sql,NULL,sizeof(sql));
    strcat(sql,SQL_CREATE_TABLE_GROUPS);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create 'groups' table successful.");

    //create groupmember table
    memset(sql,NULL,sizeof(sql));
    strcat(sql,SQL_CREATE_TABLE_GROUPMEMBER);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));
        return -1;
    }
    puts("Create 'groupmember' table successful.");

    return 0;
}

int db_connect_to_server()
{
    int ret;
    char sql[SQL_MAX_SIZE],*ptmp=NULL;

    if(mysql_init(&hMySQLServerConnection)==NULL)
    {
        printf("mysql_init failed.\n");
        return -1;
    }
    if(!mysql_real_connect(&hMySQLServerConnection,DB_HOST, DB_USERNAME, DB_PASSWORD,NULL, DB_PORT, NULL, CLIENT_FOUND_ROWS))
    {
        printf("MySQL connect failed.Error Code:%d\n",ret);
        return -2;
    }
    printf("connect to database successful.\n");
    pthread_mutex_init(&MutexMySQLConn,NULL);

    //mysql_query(&hMySQLServerConnection,"drop database ICQ;");
    mysql_query(&hMySQLServerConnection,"set names utf8;");

    //set interactive_timeout= 31536000 ;
    //set wait_timeout=31536000;
    mysql_query(&hMySQLServerConnection,"set interactive_timeout= 31536000;");
    mysql_query(&hMySQLServerConnection,"set wait_timeout=31536000;");

    //change db
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_CHANGE_USE_DB,DB_DATABASE);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts(mysql_error(&hMySQLServerConnection));

        ptmp=strstr(mysql_error(&hMySQLServerConnection),"Unknown database");
        if(ptmp==NULL)
            return -3;
        if(db_create_database()!=0)
            return -4;
    }

    return 0;
}

int db_create_num(char *numBuffer,unsigned int bufferSize)
{
    int ret;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pMySQLResults=NULL;
    MYSQL_ROW rowResults;
    unsigned int rowNum,lastNum;

    memset(numBuffer,NULL,bufferSize);

    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,SQL_QUERY_USERS_COUNT);
    if(ret!=0)
    {
        printf("Fetch users count failed.Error:");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    if(pMySQLResults==NULL)
    {
        printf("mysql_store_result() error.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    rowResults=mysql_fetch_row(pMySQLResults);
    rowNum=atoi(rowResults[0]);
    mysql_free_result(pMySQLResults);
    printf("Row Count:%d\n",rowNum);
    if(rowNum<1)
    {
        strcat(numBuffer,"10000");
        pthread_mutex_unlock(&MutexMySQLConn);
        return 0;
    }

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_USERS_LAST_DATA,rowNum-1);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("Fetch users last data failed.Error:");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pMySQLResults==NULL)
    {
        printf("mysql_store_result() error.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    rowResults=mysql_fetch_row(pMySQLResults);
    lastNum=atol(rowResults[0]);
    printf("%d\n",lastNum);
    lastNum++;
    sprintf(numBuffer,"%d",lastNum);

    return 0;
}

int db_register_account(char *num,char *nickname,char *password,char *truename,char *school,int sex,int age)
{
    int ret;
    char sql[SQL_MAX_SIZE];

    memset(num,NULL,sizeof(ID_MAXIMUM_SIZE));

    memset(sql,NULL,sizeof(sql));
    if(db_create_num(num,sizeof(num))!=0)
    {
        //add account failed
        return -1;
    }
    sprintf(sql,SQL_INSERT_TABLE_USERS,num,password,nickname,truename,school,"",sex,age);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        printf("Add '%s' account failed.",num);
        puts(mysql_error(&hMySQLServerConnection));
    }
    else
    {
        printf("Add '%s' account successfully.\n",num);
    }


    return ret;
}

int db_auth_account(ACCOUNT_INFO_DATA *pAccountInfo)
{
    int ret,rowNum;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pMySQLResults=NULL;
    MYSQL_ROW rowResults;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_AUTH_ACCOUNT,pAccountInfo->num,pAccountInfo->password);
    puts(sql);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("exec auth account SQL failed.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pMySQLResults)
    {
        rowNum=mysql_num_rows(pMySQLResults);
        if(rowNum<1)
        {
            //auth failed
            mysql_free_result(pMySQLResults);
            return -2;
        }
        rowResults=mysql_fetch_row(pMySQLResults);
        memset(pAccountInfo->groupList,NULL,sizeof(pAccountInfo->groupList));
        strcat(pAccountInfo->groupList,rowResults[7]);
        db_find_friend(pAccountInfo);
    }
    else
        return -1;

    mysql_free_result(pMySQLResults);

    return 0;
}

int db_find_friend(ACCOUNT_INFO_DATA *pAccountInfo)
{
    int ret;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pMySQLResults=NULL;
    MYSQL_ROW rowResults;
    int i,rowNum,fieldNum;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_FIND_ACCOUNT,pAccountInfo->num);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("exec find account SQL failed.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pMySQLResults==NULL)
    {
        printf("mysql_store_result error.\n");
        mysql_free_result(pMySQLResults);
        return -2;
    }
    rowNum=mysql_num_rows(pMySQLResults);
    fieldNum=mysql_num_fields(pMySQLResults);
    if(rowNum<1)
    {
        printf("Can not find account '%s'\n",pAccountInfo->num);
        mysql_free_result(pMySQLResults);
        return -2;     //not find
    }
    rowResults=mysql_fetch_row(pMySQLResults);
    strcat(pAccountInfo->nickname,rowResults[2]);
    strcat(pAccountInfo->truename,rowResults[3]);
    strcat(pAccountInfo->school,rowResults[4]);
    strcat(pAccountInfo->signature,rowResults[5]);
    pAccountInfo->sex=atoi(rowResults[6]);
    pAccountInfo->age=atoi(rowResults[7]);
    pAccountInfo->success=1;
    printf("fetch account '%s' successful.\n",pAccountInfo->num);

    mysql_free_result(pMySQLResults);

    return 0;
}

int db_add_friendship(FRIENDSHIP_INFO_DATA *pFriendshipInfo)
{
    int ret;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pResults=NULL;
    int rowNum;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_FRIENDSHIP,pFriendshipInfo->num1,pFriendshipInfo->num2);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("add friendship for '%s' and '%s' failed.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    if(pResults==NULL)
    {
        printf("add friendship for '%s' and '%s' failed.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        return -2;
    }
    pthread_mutex_unlock(&MutexMySQLConn);
    rowNum=mysql_num_rows(pResults);
    mysql_free_result(pResults);
    if(rowNum>0)
    {
        printf("'%s' and '%s' failed is already friend.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        return -3;
    }

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_FRIENDSHIP,pFriendshipInfo->num2,pFriendshipInfo->num1);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("add friendship for '%s' and '%s' failed.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    if(pResults==NULL)
    {
        printf("add friendship for '%s' and '%s' failed.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        return -2;
    }
    pthread_mutex_unlock(&MutexMySQLConn);
    rowNum=mysql_num_rows(pResults);
    mysql_free_result(pResults);
    if(rowNum>0)
    {
        printf("'%s' and '%s' failed is already friend.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        return -3;
    }


    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_INSERT_TABLE_FRIENDSHIP,pFriendshipInfo->num1,pFriendshipInfo->num2,pFriendshipInfo->group1,\
            pFriendshipInfo->group2);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        printf("add friendship for '%s' and '%s' failed.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);
        return -2;
    }
    printf("add friendship for '%s' and '%s' successful.\n",pFriendshipInfo->num1,pFriendshipInfo->num2);

    return 1;
}

char *db_search_friend_list(char *num)
{
    char *pFriendList=NULL,sql[SQL_MAX_SIZE];
    MYSQL_RES *pResults=NULL;
    MYSQL_ROW rowRsults;
    int ret,rowNum,i,flag=1;

    pFriendList=(char *)malloc(5000);
    if(pFriendList==NULL)
    {
        printf("malloc error.\n");
        return -1;
    }
    memset(pFriendList,NULL,5000);
    pthread_mutex_lock(&MutexMySQLConn);
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_FRIENDSHIP_2,"num1",num);
skip_1:
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("exec SQL failed.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    if(pResults==NULL)
    {
        printf("mysql_store_result() error.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    rowNum=mysql_num_rows(pResults);
    if(rowNum>0)
    {
        for(i=0; i<rowNum; i++)
        {
            rowRsults=mysql_fetch_row(pResults);
            if(flag)
                strcat(pFriendList,rowRsults[1]);
            else
                strcat(pFriendList,rowRsults[0]);
            strcat(pFriendList,";");
        }
    }
    mysql_free_result(pResults);
    if(flag)
    {
        flag=0;
        memset(sql,NULL,sizeof(sql));
        sprintf(sql,SQL_QUERY_FRIENDSHIP_2,"num2",num);
        goto skip_1;
    }
    pthread_mutex_unlock(&MutexMySQLConn);
    if(strlen(pFriendList)<1)
    {
        printf("friend list is empty.\n");
        free(pFriendList);
        return NULL;
    }
    printf("friend list is not empty.\n");

    return pFriendList;
}

int db_update_account_data(ACCOUNT_INFO_DATA *pAccountData)
{
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pResults=NULL;
    int ret;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_UPDATE_ACCOUNT_DATA,pAccountData->nickname,pAccountData->truename,pAccountData->school,\
            pAccountData->signature,pAccountData->sex,pAccountData->age,pAccountData->num);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        printf("update account data failed.\n");
        return -1;
    }
    if(strlen(pAccountData->password)>0)
    {
        memset(sql,NULL,sizeof(sql));
        sprintf(sql,SQL_UPDATE_ACCOUNT_PASSWORD,pAccountData->password,pAccountData->num);
        pthread_mutex_lock(&MutexMySQLConn);
        ret=mysql_query(&hMySQLServerConnection,sql);
        pthread_mutex_unlock(&MutexMySQLConn);
        if(ret!=0)
        {
            printf("update account password failed.\n");
            return -1;
        }
    }

    return 0;
}

int db_add_leave_message(SMS_DATA smsData)
{
    int ret;
    char sql[SQL_MAX_SIZE];

    memset(sql,NULL,sizeof(sql));

    sprintf(sql,SQL_INSERT_TABLE_LEAVEMESSAGES,smsData.fromNum,smsData.toNum,smsData.sms);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        printf("insert leavemessages failed.\n");
        return -1;
    }

    return 0;
}

int db_query_leave_message(SMS_DATA *smsData)
{
    int ret,rowNum;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pResults=NULL;
    MYSQL_ROW rowResult;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_LEAVEMESSAGES,smsData->toNum);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    if(pResults==NULL)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    rowNum=mysql_num_rows(pResults);
    if(rowNum<1)
    {
        free(pResults);
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    rowResult=mysql_fetch_row(pResults);
    memset(smsData->fromNum,NULL,sizeof(smsData->fromNum));
    memset(smsData->sms,NULL,sizeof(smsData->sms));
    strcat(smsData->fromNum,rowResult[0]);
    strcat(smsData->sms,rowResult[2]);
    free(pResults);

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_DELETE_LEAVEMESSAGES,smsData->toNum);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0) return -1;

    return 0;
}

int db_delete_friend(char *num1,char *num2)
{
    char sql[SQL_MAX_SIZE];
    int ret;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_DELETE_FRIENDSHIP,num1,num2);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        return -1;
    }
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_DELETE_FRIENDSHIP,num2,num1);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        return -1;
    }

    return 0;
}

int db_create_group_num(char *numBuffer,unsigned int bufferSize)
{
    int ret;
    char sql[SQL_MAX_SIZE];
    MYSQL_RES *pMySQLResults=NULL;
    MYSQL_ROW rowResults;
    unsigned int rowNum,lastNum;

    memset(numBuffer,NULL,bufferSize);

    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,SQL_QUERY_GROUPS_COUNT);
    if(ret!=0)
    {
        printf("Fetch groups count failed.Error:");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    if(pMySQLResults==NULL)
    {
        printf("mysql_store_result() error.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    rowResults=mysql_fetch_row(pMySQLResults);
    rowNum=atoi(rowResults[0]);
    mysql_free_result(pMySQLResults);
    if(rowNum<1)
    {
        strcat(numBuffer,"10000");
        pthread_mutex_unlock(&MutexMySQLConn);
        return 0;
    }

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_GROUPS_LAST_DATA,rowNum-1);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        printf("Fetch groups last data failed.Error:");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pMySQLResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pMySQLResults==NULL)
    {
        printf("mysql_store_result() error.\n");
        pthread_mutex_unlock(&MutexMySQLConn);
        return -2;
    }
    rowResults=mysql_fetch_row(pMySQLResults);
    lastNum=atol(rowResults[0]);
    lastNum++;
    sprintf(numBuffer,"%d",lastNum);

    return 0;
}

int db_create_group(CREATE_GROUP_DATA *pCreateGroupData)
{
    char sql[SQL_MAX_SIZE];
    char groupNum[ID_MAXIMUM_SIZE];
    int ret;

    memset(sql,NULL,sizeof(sql));
    if(db_create_group_num(groupNum,sizeof(groupNum))!=0)
    {
        puts("create group num failed.");
        return -1;
    }
    sprintf(sql,SQL_INSERT_TABLE_GROUPS,groupNum,pCreateGroupData->groupName,pCreateGroupData->groupIntro);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(ret!=0)
    {
        puts("insert new group item error.");
        return -1;
    }
    memset(pCreateGroupData->groupNum,NULL,sizeof(pCreateGroupData->groupNum));
    strcat(pCreateGroupData->groupNum,groupNum);

    return 0;
}

int db_find_group(GROUP_INFO_DATA *pGroupInfoData)
{
    char sql[SQL_MAX_SIZE];
    int ret,rowNum;
    MYSQL_RES *pResults=NULL;
    MYSQL_ROW rowData;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_GROUPS,pGroupInfoData->groupNum);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        pGroupInfoData->flag=-1;
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        return -1;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pResults==NULL)
    {
        pGroupInfoData->flag=-1;
        return -2;
    }
    rowNum=mysql_num_rows(pResults);
    if(rowNum<1)
    {
        pGroupInfoData->flag=0;
        mysql_free_result(pResults);
        return 0;
    }
    rowData=mysql_fetch_row(pResults);
    strcat(pGroupInfoData->groupName,rowData[1]);
    strcat(pGroupInfoData->groupIntro,rowData[2]);
    pGroupInfoData->flag=1;
    mysql_free_result(pResults);

    return 0;
}

int db_join_group(GROUP_INFO_DATA *pGroupInfoData,char *memberID)
{
    char sql[SQL_MAX_SIZE];
    int ret,rowNum;
    MYSQL_RES *pResults=NULL;

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_EXIST_GROUPMEMBER,pGroupInfoData->groupNum,memberID);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts("query error.");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        pGroupInfoData->flag=-2;
        return -1;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    if(pResults==NULL)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        pGroupInfoData->flag=-2;
        return -1;
    }
    rowNum=mysql_num_rows(pResults);
    mysql_free_result(pResults);
    if(rowNum>0)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        pGroupInfoData->flag=-1;
        return 0;
    }
    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_INSERT_TABLE_GROUPMEMBER,pGroupInfoData->groupNum,memberID);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        puts("insert error.");
        puts(mysql_error(&hMySQLServerConnection));
        pthread_mutex_unlock(&MutexMySQLConn);
        pGroupInfoData->flag=-2;
        return -1;
    }
    pGroupInfoData->flag=1;
    pthread_mutex_unlock(&MutexMySQLConn);

    return 0;
}

char *db_search_group_list(char *accountNum)
{
    char *pGroupList=NULL;
    char sql[SQL_MAX_SIZE];
    int i,ret,rowNum;
    MYSQL_RES *pResults=NULL;
    MYSQL_ROW rowData;

    pGroupList=(char *)malloc(1000);
    if(pGroupList==NULL)
        return NULL;
    memset(pGroupList,NULL,1000);

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_GROUPMEMBER,accountNum);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        puts(mysql_error(&hMySQLServerConnection));
        return NULL;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pResults==NULL)
        return NULL;
    rowNum=mysql_num_rows(pResults);
    if(rowNum<1)
    {
        mysql_free_result(pResults);
        return NULL;
    }

    for(i=0;i<rowNum;i++)
    {
        rowData=mysql_fetch_row(pResults);
        strcat(pGroupList,rowData[0]);
        strcat(pGroupList,";");
    }
    mysql_free_result(pResults);

    return pGroupList;
}

char *db_search_group_member(char *pGroupNum)
{
    char *pGroupMember=NULL;
    char sql[SQL_MAX_SIZE];
    int i,ret,rowNum;
    MYSQL_RES *pResults=NULL;
    MYSQL_ROW rowData;

    pGroupMember=(char *)malloc(1000);
    if(pGroupMember==NULL)
        return NULL;
    memset(pGroupMember,NULL,1000);

    memset(sql,NULL,sizeof(sql));
    sprintf(sql,SQL_QUERY_GROUPMEMBER_BY_GROUPNUM,pGroupNum);
    pthread_mutex_lock(&MutexMySQLConn);
    ret=mysql_query(&hMySQLServerConnection,sql);
    if(ret!=0)
    {
        pthread_mutex_unlock(&MutexMySQLConn);
        puts(mysql_error(&hMySQLServerConnection));
        return NULL;
    }
    pResults=mysql_store_result(&hMySQLServerConnection);
    pthread_mutex_unlock(&MutexMySQLConn);
    if(pResults==NULL)
        return NULL;
    rowNum=mysql_num_rows(pResults);
    if(rowNum<1)
    {
        mysql_free_result(pResults);
        return NULL;
    }

    for(i=0;i<rowNum;i++)
    {
        rowData=mysql_fetch_row(pResults);
        strcat(pGroupMember,rowData[1]);
        strcat(pGroupMember,";");
    }
    mysql_free_result(pResults);

    return pGroupMember;
}









