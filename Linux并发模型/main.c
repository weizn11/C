#include <stdio.h>
#include <stdlib.h>

#include "async.h"
#include "mempool.h"



int main(int argc,char *argv[])
{
    int i;
    int pid;

    pid=fork();
    if(pid<0)
        printf("fork() error\n");
    else if(pid>0)
    {
        printf("father process exit\n");
        exit(0);
    }
    setsid();
    listen_client();

    return 0;
}
