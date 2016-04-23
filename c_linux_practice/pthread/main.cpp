#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_cond_t cond;
pthread_mutex_t mutex;

int pthread_init()
{
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    return 0;
}

void *new_thread(void *Parameter)
{
    int n;
    int num=*(int *)Parameter;

    while(1)
    {
        pthread_mutex_lock(&mutex);
        //进入条件变量时阻塞线程，释放互斥锁
        pthread_cond_wait(&cond,&mutex);
        //离开条件变量时获取互斥锁
        pthread_mutex_unlock(&mutex);

        for(n=0;n<10;n++)
        {
            printf("I'm %d.\n",num);
            sleep(1);
        }
    }

    return NULL;
}

int start_thread()
{
    pthread_mutex_lock(&mutex);
    //释放一个条件变量
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int main()
{
    int n;
    pthread_t threadID;
    char ch;

    pthread_init();
    for(n=0;n<10;n++)
    {
        pthread_create(&threadID,NULL,new_thread,(void *)&n);
        usleep(1000);
    }
    while(1)
    {
        fflush(stdin);
        scanf("%c",&ch);
        switch(ch)
        {
        case 'r':
            start_thread();
            break;
        case 'a':
            pthread_mutex_lock(&mutex);
            //释放所有的条件变量
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    return 0;
}









