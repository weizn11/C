#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

using namespace std;

int main()
{
    int fd_w,fd_r;
    int fproc;
    char strBuffer[PIPE_BUF];

    memset(strBuffer,NULL,sizeof(strBuffer));

    if(mkfifo("FIFO",0777)!=0 && errno!=EEXIST)
    {
        cout <<"create FIFO failed."<<endl;
        return -1;
    }
    cout <<"create FIFO successful."<<endl;
    fproc=fork();
    if(fproc<0)
    {
        cout <<"create child process failed."<<endl;
        return -2;
    }
    if(fproc==0)
    {
        fd_r=open("FIFO",O_RDONLY);
        if(fd_r==-1)
        {
            cout <<"read FIFO failed."<<endl;
            return -3;
        }
        sleep(2);
        while(read(fd_r,strBuffer,sizeof(strBuffer)-1))
        {
            cout <<strBuffer;
            memset(strBuffer,NULL,sizeof(strBuffer));
        }
        cout <<"read over."<<endl;
        exit(0);
    }
    fd_w=open("FIFO",O_WRONLY);
    if(fd_w==-1)
    {
        cout <<"write FIFO failed."<<endl;
        return -4;
    }
    strcat(strBuffer,"Hello\n");
    int count=10;
    while(count--)
    {
        write(fd_w,strBuffer,strlen(strBuffer));
    }
    close(fd_w);
    cout <<"write over."<<endl;

    return 0;
}





