#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mysql/mysql.h>

#include "p2p/p2p.h"
#include "concurrent/async.h"

int main()
{
    pthread_t threadID;

    pthread_create(&threadID,NULL,listen_client,NULL);

    p2p_listen_server();

    return 0;
}
