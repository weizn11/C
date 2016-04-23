#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <mysql/mysql.h>

#include "p2p/p2p.h"
#include "concurrent/async.h"
#include "db/db.h"

int main()
{
    char str[ID_MAXIMUM_SIZE];
    pthread_t threadID;

    printf("Launch...\n");
    if(db_connect_to_server()!=0)
    {
        return -1;
    }

    if(access("images/icon",0)!=0)
    {
        if(mkdir("images", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0)
        {
            printf("mkdir error.\n");
            return -1;
        }
        if(mkdir("images/icon", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0)
        {
            printf("mkdir error.\n");
            return -1;
        }
        printf("mkdirs images/icon successful.\n");
    }
    pthread_create(&threadID,NULL,listen_client,NULL);

    p2p_listen_server();

    return 0;
}
