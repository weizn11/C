#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

using namespace std;

void signal_print(int signo)
{
    cout <<"signal code:"<<signo<<endl;
    return;
}

int test_1()
{
    signal(SIGINT,signal_print);
    return 0;
}

int test_2()
{
    signal(SIGALRM,signal_print);
    alarm(3);
    return 0;
}

int test_3()
{
    signal(SIGUSR1,signal_print);
    return 0;
}

void signal_test_4(int signo)
{
    signal_print(signo);
    cout <<"The child process will be exit."<<endl;
    exit(0);
    return;
}

int test_4()
{
    int fd;

    fd=fork();
    if(fd==0)
    {
        signal(SIGUSR1,signal_test_4);
        while(1)
        {
            cout <<"My PID is "<<getpid()<<endl;
            sleep(1);
        }
    }
    sleep(5);
    kill(fd,SIGUSR1);

    return 0;
}

int main()
{
    test_1();
    test_2();
    test_3();
    test_4();
    kill(getpid(),SIGUSR1);
    pause();
    return 0;
}
