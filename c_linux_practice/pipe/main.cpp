#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

using namespace std;

int main()
{
    int fpw[2],fpr[2];
    int fpro,fpro2;
    char strBuffer[PIPE_BUF];

    if(pipe(fpw)!=0 || pipe(fpr)!=0)
    {
        cout <<"create pipe failed."<<endl;
        return -1;
    }

    fpro=fork();
    if(fpro<0)
    {
        cout <<"create child process failed."<<endl;
        return -1;
    }
    if(fpro==0)
    {
        close(fpw[1]);
        close(fpr[0]);

        dup2(fpw[0],STDIN_FILENO);
        dup2(fpr[1],STDOUT_FILENO);

        execl("/bin/sh",NULL);
    }

    fpro2=fork();
    if(fpro2<0)
    {
        cout <<"create child process failed."<<endl;
        kill(fpro,SIGKILL);
        return -1;
    }
    if(fpro2==0)
    {
        while(1)
        {
            memset(strBuffer,NULL,sizeof(strBuffer));
            if(read(fpr[0],strBuffer,sizeof(strBuffer)-1)<=0)
            {
                cout <<"read pipe closed."<<endl;
                close(fpr[0]);
                exit(0);
            }
            cout <<strBuffer;
        }
    }
    close(fpw[0]);
    close(fpr[1]);
    close(fpr[0]);

    while(1)
    {
        memset(strBuffer,NULL,sizeof(strBuffer));
        cin.getline(strBuffer,sizeof(strBuffer)-1);
        strcat(strBuffer,"\n");
        if(write(fpw[1],strBuffer,strlen(strBuffer))<=0)
        {
            cout <<"write pipe closed."<<endl;
            kill(fpro,SIGKILL);
            close(fpw[1]);
            break;
        }
    }

    return 0;
}





