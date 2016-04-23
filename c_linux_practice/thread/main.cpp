#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

using namespace std;

pthread_t thread2;

void *new_thread_2(void *parameter)
{
    while(1)
    {
        sleep(1);
        cout <<"I'm thread 2"<<endl;
    }
    return NULL;
}

void *new_thread_1(void *parameter)
{
    sleep(3);
    pthread_cancel(thread2);
    sleep(2);

    return 0;
}

int main()
{
    pthread_t thread1;
    void *retval=NULL;

    pthread_create(&thread1,NULL,new_thread_1,NULL);
    pthread_create(&thread2,NULL,new_thread_2,NULL);

    if(pthread_join(thread1,&retval)!=0)
    {
        cout <<"pthread_join error."<<endl;
        return -1;
    }
    else
    {
        cout <<"thread 1 exit."<<endl;
    }

    if(pthread_join(thread2,&retval)!=0)
    {
        cout <<"pthread_join error."<<endl;
        return -1;
    }
    else
    {
        cout <<"thread 2 exit."<<endl;
    }

    return 0;
}
