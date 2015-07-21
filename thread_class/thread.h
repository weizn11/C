#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include <windows.h>

class Thread
{
public:
    Thread();
    int run(void *Parameter);
protected:
    virtual int func(void *threadPara)=0;
    static DWORD WINAPI start_thread(LPVOID Parameter);
protected:
    void *threadPara;
    int runFlag;
};

#endif // THREAD_H_INCLUDED
