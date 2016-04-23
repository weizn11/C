#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

#include "../concurrent/async.h"

#define DB_HOST "127.0.0.1"
#define DB_USERNAME "root"
#define DB_PASSWORD "305859616"
#define DB_PORT 3306
#define DB_DATABASE "ICQ"
#define SQL_MAX_SIZE 3000

#define SQL_CHANGE_USE_DB "use %s;"
#define SQL_CREATE_DATABASE "create database %s;"
#define SQL_SET_CHARACTER "alter database %s  character set utf8;"
#define SQL_CREATE_TABLE_USERS "create table users(\
                                                   num VARCHAR(20) not null primary key,\
                                                   password VARCHAR(30) not null,\
                                                   nickname VARCHAR(100),\
                                                   turename VARCHAR(100),\
                                                   school VARCHAR(100),\
                                                   signature VARCHAR(150),\
                                                   sex INT(4),\
                                                   age INT(4),\
                                                   groupList VARCHAR(500));"
#define SQL_CREATE_TABLE_FRIENDSHIP "create table friendship(\
                                                             num1 VARCHAR(20) not null,\
                                                             num2 VARCHAR(20) not null,\
                                                             group1 VARCHAR(50),\
                                                             group2 VARCHAR(50));"
#define SQL_CREATE_TABLE_LEAVEMESSAGES "create table leavemessages(\
                                                                   fromNum VARCHAR(20) not null,\
                                                                   toNum VARCHAR(20) not null,\
                                                                   message VARCHAR(2000) not null);"
#define SQL_CREATE_TABLE_GROUPS "create table groups(\
                                                     num VARCHAR(20) not null primary key,\
                                                     name VARCHAR(100) not null,\
                                                     intro VARCHAR(200));"
#define SQL_CREATE_TABLE_GROUPMEMBER "create table groupmember(\
                                                               groupNum VARCHAR(20) not null,\
                                                               memberNum VARCHAR(20) not null);"

#define SQL_INSERT_TABLE_USERS "insert into users values(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,\"\");"
#define SQL_INSERT_TABLE_LEAVEMESSAGES "insert into leavemessages values('%s','%s','%s');"
#define SQL_INSERT_TABLE_GROUPS "insert into groups values('%s','%s','%s');"
#define SQL_INSERT_TABLE_GROUPMEMBER "insert into groupmember values('%s','%s');"
#define SQL_INSERT_TABLE_FRIENDSHIP "insert into friendship values(\"%s\",\"%s\",\"%s\",\"%s\");"

#define SQL_AUTH_ACCOUNT "select * from users where num=\"%s\" and password=\"%s\";"

#define SQL_QUERY_USERS_COUNT "select count(*) from users;"
#define SQL_QUERY_GROUPS_COUNT "select count(*) from groups;"
#define SQL_QUERY_USERS_LAST_DATA "select num from users limit %d,1;"
#define SQL_QUERY_GROUPS_LAST_DATA "select num from groups limit %d,1;"
#define SQL_QUERY_FRIENDSHIP "select * from friendship where num1='%s' and num2='%s';"
#define SQL_QUERY_FRIENDSHIP_2 "select * from friendship where %s='%s';"
#define SQL_QUERY_GROUPS "select * from groups where num='%s';"
#define SQL_QUERY_EXIST_GROUPMEMBER "select * from groupmember where groupNum='%s' and memberNum='%s';"
#define SQL_QUERY_GROUPMEMBER "select * from groupmember where memberNum='%s';"
#define SQL_QUERY_GROUPMEMBER_BY_GROUPNUM "select * from groupmember where groupNum='%s';"
#define SQL_QUERY_LEAVEMESSAGES "select * from leavemessages where toNum='%s' limit 0,1;"

#define SQL_FIND_ACCOUNT "select * from users where num='%s';"

#define SQL_UPDATE_ACCOUNT_DATA "update users set nickname='%s',turename='%s',school='%s',signature='%s',sex=%d,\
                                age=%d where num='%s';"
#define SQL_UPDATE_ACCOUNT_PASSWORD "update users set password='%s' where num='%s';"

#define SQL_DELETE_LEAVEMESSAGES "delete from leavemessages where toNum='%s' limit 1;"
#define SQL_DELETE_FRIENDSHIP "delete from friendship where num1='%s' and num2='%s';"


int db_connect_to_server();
int db_auth_account(ACCOUNT_INFO_DATA *pAccountInfo);
int db_register_account(char *num,char *nickname,char *password,char *truename,char *school,int sex,int age);
int db_query_leave_message(SMS_DATA *smsData);
int db_add_leave_message(SMS_DATA smsData);

#endif // DB_H_INCLUDED
