#include <iostream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

int main()
{
    int fd;

    fd=vfork();
    if(fd==0)
    {
        execl("/bin/ls","ls","-al",NULL);   //若调用进程由vfork()创建，exec()返回老的空间给父进程，父进程不再阻塞。
        while(1);
        exit(0);
    }

    cout <<"Hello"<<endl;   //此段代码可被执行

    return 0;
}
