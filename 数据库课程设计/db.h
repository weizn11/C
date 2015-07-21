#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

#define DB_HOST "202.201.1.60"
#define DB_USERNAME "root"
#define DB_PASSWORD ""
#define DB_DATABASE "Library"
#define DB_PORT 3306

#include <mysql.h>

int connect_to_database();

#endif // DB_H_INCLUDED
